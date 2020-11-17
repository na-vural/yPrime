#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>
#include <pthread.h>
#include <sys/sysinfo.h>


struct thread_data
{
        mpz_t *num;
        mpz_t *limit;
        signed char *check_byt;
        unsigned char thread_id;
        unsigned char *num_of_threads;
        unsigned short int inc_seq_index;
        unsigned char *inc_seq;
};


void create_eliminated_seq( unsigned char inc_seq[],
                            unsigned short int inc_seq_index,
                            unsigned char primes_array[],
                            unsigned char primes_index)
{
        /* To create an addition sequence 
         * without multiples of n odd primes. */
        unsigned char mod_result[primes_index];
        unsigned char check;
        unsigned short int i, j;

        /* Create mod list to specify in which steps we come up with
         * the numbers that are divisible with our elimination primes.
         * We just seek in first n'th steps (n is primes_index), 
         * and find 'the step number' modulo 'our primes'.
         * For example if we come up with 6 in the step 5 
         * then we come up with 9 in the step 8, namely every step 
         * that has 2 as modulo 3 is divisible with 3. Like this,
         * we find the step of the first divisible number with 
         * our i'th prime. Then calculate 'this step' modulo 
         * 'ith primes value'. Then we save this modulus result 
         * to the mod_result array. The mod_result array's 
         * i'th index holds the modulus result of the 
         * primes_array's i'th prime. And repeat this algorithm
         * until find the all modulus of the our elimination primes.*/
        unsigned short int step = 1; /* First step for addition */
        
        unsigned char addend = 2; /* The value of the addition in an each step. 
                                   * It is 2 because we just count odd numbers.*/
        
        for (i = 0 ; i < primes_index ; ) {
                if ( (1 + addend) % primes_array[i] == 0 ) {
                        mod_result[i] = step;
                        step++;
                        addend += 2;
                        i++;
                } else {
                        step++;
                        addend += 2;
                }
        }

        /* Finding the inc value to jump over the numbers
         * that divisible with our elimination primes.
         * The mod list created above is used in this algorithm.
         * Simply, we check the n'th step and */
        step = 1;
        for (i = 0 ; i < inc_seq_index ; i++) {
                addend = 2;
                check = 0;
                while (check < primes_index) {
                        for (j = 0 ; j < primes_index ; j++) {
                            if (step % primes_array[j] == mod_result[j]) {
                                    step++;
                                    addend += 2;
                                    check = 0;
                            } else {
                                    check++;
                            }
                        }
                }
                inc_seq[i] = addend;
                step++;
        }
}


void *check( void *threadargs )
{
        struct thread_data *args = (struct thread_data*) threadargs;
        mpz_t *num = args->num;
        mpz_t *limit = args->limit;
        signed char *check_byt = args->check_byt;
        unsigned char thread_id = args->thread_id;
        unsigned char *num_of_threads = args->num_of_threads;
        unsigned char *inc_seq = args->inc_seq;
        unsigned short int inc_seq_index = args->inc_seq_index;
 
        mpz_t divisor;
        mpz_init(divisor);
        mpz_set_ui(divisor, 1);

        if ( thread_id == 0 ) {
                thread_id = *num_of_threads;
        }

        unsigned char step_init = 0;
        for ( unsigned int i = 0 ; i < thread_id ; i++ ) {
                step_init += inc_seq[i];
        }
        
        mpz_add_ui(divisor, divisor, step_init);

        unsigned char parallel_inc[inc_seq_index];
        for ( unsigned int i = 0 ; i < inc_seq_index ; i++ ) {
            parallel_inc[i] = 0;
            for ( unsigned char j = 0 ; j < *num_of_threads ; j++ ) {
                parallel_inc[i] 
                +=
                inc_seq[(i * (*num_of_threads) + thread_id + j) % inc_seq_index];
            }
        }

        for ( unsigned char i = 0 ; mpz_cmp(*limit, divisor) >= 0 ; i++ ) {
                i %= inc_seq_index;
                if ( mpz_divisible_p(*num, divisor) != 0 ) {
                        gmp_printf("Not prime. Divisible with %Zd", divisor);
                        exit(0);
                }
                mpz_add_ui(divisor, divisor, parallel_inc[i]);
        }

        return 0;
}



int main(int argc, char *argv[]) 
{
        mpz_t num;
        
        if ( (argc != 2) || (mpz_init_set_str(num, argv[1], 10) != 0) ) {
                printf("%s", "\nUsage: yprime <number>\n\n");
                return -1;
        }

//        mpz_init(num);
//        printf("%s", "Gimme the integer\n---> ");
//        gmp_scanf("%Zd", num);
        gmp_printf("Given number is %Zd\n", num);

        if ( mpz_divisible_ui_p(num, 2) != 0 ) {
                if ( mpz_cmp_ui(num, 2) == 0 ) {
                        printf("%s", "First and last even prime.");
                        return 0;
                } else {
                        printf("%s", "Not prime. It is an even number.");
                        return 0;
                }
        }

        unsigned char el_primes[] = {3, 5, 7, 11};
        unsigned char el_primes_index = 4;
        for ( unsigned char i = 0; i < 4 ; i++ ) {
                if ( mpz_divisible_ui_p(num, el_primes[i]) != 0 ) {
                        if ( mpz_cmp_ui(num, el_primes[i]) == 0 ) {
                                printf("%s", "Prime");
                                return 0;
                        } else {
                                printf( "Not prime divisible with %d",
                                        el_primes[i]);
                                return 0;
                        }
                }
        }

        unsigned short int inc_seq_index = 480;
        unsigned char inc_seq[inc_seq_index];
        create_eliminated_seq(  inc_seq,
                                inc_seq_index,
                                el_primes,
                                el_primes_index);

        mpz_t limit;
        mpz_init(limit);
        mpz_sqrt(limit, num);

        unsigned char num_of_threads = get_nprocs();

        pthread_t threads[num_of_threads];
        signed char check_byt = 0;

        struct thread_data threadargs[num_of_threads];
        for ( unsigned char i = 0 ; i < num_of_threads ; i++ ) {
                threadargs[i].num = &num;
                threadargs[i].limit = &limit;
                threadargs[i].check_byt = &check_byt;
                threadargs[i].thread_id = i;
                threadargs[i].num_of_threads = &num_of_threads;
                threadargs[i].inc_seq_index = inc_seq_index;
                threadargs[i].inc_seq = inc_seq;
                pthread_create( &threads[i],
                                NULL,
                                check,
                                (void *) &threadargs[i]);
        }

        void *thread_ret;

        for ( unsigned char i = 0 ; i < num_of_threads ; i++ ) {
                pthread_join(threads[i], &thread_ret); 
        }

        //pthread_join(threads[0], NULL);
        //check(num, limit, 1, inc_seq, 480);
        printf("%s", "PRIME");
        pthread_exit(NULL);
        return 0;
}
