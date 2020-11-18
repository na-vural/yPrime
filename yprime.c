#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>
#include <pthread.h>
#include <sys/sysinfo.h>


struct thread_data
{
        mpz_t *num;
        mpz_t *limit;
        unsigned char thread_id;
        unsigned char *num_of_threads;
        unsigned short int inc_seq_len;
        unsigned char *inc_seq;
};


void create_eliminated_seq( unsigned char inc_seq[],
                            unsigned short int inc_seq_len,
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
        
        unsigned char inc_value = 2; /* The value of the addition in an each step. 
                                   * It is 2 because we just count odd numbers.*/
        
        for (i = 0 ; i < primes_index ; ) {
                if ( (1 + inc_value) % primes_array[i] == 0 ) {
                        mod_result[i] = step;
                        step++;
                        inc_value += 2;
                        i++;
                } else {
                        step++;
                        inc_value += 2;
                }
        }

        /* To find the inc value to jump over the numbers
         * that divisible with our elimination primes.
         * The mod list created above is used in this algorithm.
         * Simply, we check the n'th step and decide whether
         * this step is divisible with our primes or not.
         * If a step is divisible, then we add 2 to inc value,
         * and look at next step. If the next step also divisible
         * we again add 2 to inc value. Until find a step that are
         * not divisible with our primes, we continue to this proccess.
         * When find a such step that non-divisible, we just store
         * the current inc value, and initialize this inc value for
         * the proccess continuation. To simplify this, we created 
         * and use mod_result array. */
        step = 1;
        for (i = 0 ; i < inc_seq_len ; i++) {
                inc_value = 2;
                check = 0;
                while (check < primes_index) {
                        for (j = 0 ; j < primes_index ; j++) {
                            if (step % primes_array[j] == mod_result[j]) {
                                    step++;
                                    inc_value += 2;
                                    check = 0;
                            } else {
                                    check++;
                            }
                        }
                }
                inc_seq[i] = inc_value;
                step++;
        }
}


void *check( void *threadargs )
{
        struct thread_data *args = (struct thread_data*) threadargs;
        mpz_t *num = args->num;
        mpz_t *limit = args->limit;
        unsigned char thread_id = args->thread_id;
        unsigned char *num_of_threads = args->num_of_threads;
        unsigned char *inc_seq = args->inc_seq;
        unsigned short int inc_seq_len = args->inc_seq_len;
 
        mpz_t divisor;
        mpz_init(divisor);
        mpz_set_ui(divisor, 1);

        /* For example if there are 4 threads,
         * zeroth, first, second, third,
         * we just convert zeroth thread to fourth
         * thread. */
        if ( thread_id == 0 ) {
                thread_id = *num_of_threads;
        }

        /* We create special steps for all threads.
         * So each of them start the division with
         * different values. */
        unsigned char step_init = 0;
        for ( unsigned int i = 0 ; i < thread_id ; i++ ) {
                step_init += inc_seq[i];
        }
        mpz_add_ui(divisor, divisor, step_init);

        /* We share out the divisor values between threads.
         * To achieve this, we create different increment steps
         * for all threads. */
        unsigned char parallel_inc[inc_seq_len];
        for ( unsigned int i = 0 ; i < inc_seq_len ; i++ ) {
            parallel_inc[i] = 0;
            for ( unsigned char j = 0 ; j < *num_of_threads ; j++ ) {
                parallel_inc[i] 
                +=
                inc_seq[(i * (*num_of_threads) + thread_id + j) % inc_seq_len];
            }
        }

        /* Here is brute force section.
         * We divide our prime candidate by all the numbers that are odd 
         * and not divisible with our elimination primes. */
        for ( unsigned char i = 0 ; mpz_cmp(*limit, divisor) >= 0 ; i++ ) {
                i %= inc_seq_len;
                if ( mpz_divisible_p(*num, divisor) != 0 ) {
                        gmp_printf("Not prime. Divisible with %Zd\n", divisor);
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
                return 1;
        }
        gmp_printf("Given number is %Zd\n", num);

        /* Eliminate 2 and even numbers. */
        if ( mpz_divisible_ui_p(num, 2) != 0 ) {
                if ( mpz_cmp_ui(num, 2) == 0 ) {
                        printf("%s", "First and last even prime.\n");
                        return 0;
                } else {
                        printf("%s", "Not prime. It is an even number.\n");
                        return 0;
                }
        }

        /* Our number are checked with elimination primes,
         * then if the number is not divisible these primes,
         * we eliminate all of them from future divisions. */
        unsigned char el_primes[] = {3, 5, 7, 11};
        unsigned char el_primes_len = 4;
        for ( unsigned char i = 0; i < el_primes_len ; i++ ) {
                if ( mpz_divisible_ui_p(num, el_primes[i]) != 0 ) {
                        if ( mpz_cmp_ui(num, el_primes[i]) == 0 ) {
                                printf("%s", "Prime\n");
                                return 0;
                        } else {
                                printf( "Not prime divisible with %d.\n",
                                        el_primes[i]);
                                return 0;
                        }
                }
        }

        /* An algorithm will be added 
         * to automatically calculate inc_seq_len. */
        unsigned short int inc_seq_len = 480;
        unsigned char inc_seq[inc_seq_len];
        create_eliminated_seq(  inc_seq,
                                inc_seq_len,
                                el_primes,
                                el_primes_len);
        
        /* Limit is the square root of our number,
         * we divide our number by all the numbers 
         * until this limit. */
        mpz_t limit;
        mpz_init(limit);
        mpz_sqrt(limit, num);

        /* Created threads number will be number of processors. */
        unsigned char num_of_threads = get_nprocs();
        pthread_t threads[num_of_threads];
        struct thread_data threadargs[num_of_threads];
        /* Thread creation. */
        for ( unsigned char i = 0 ; i < num_of_threads ; i++ ) {
                threadargs[i].num = &num;
                threadargs[i].limit = &limit;
                threadargs[i].thread_id = i;
                threadargs[i].num_of_threads = &num_of_threads;
                threadargs[i].inc_seq_len = inc_seq_len;
                threadargs[i].inc_seq = inc_seq;
                pthread_create( &threads[i],
                                NULL,
                                check,
                                (void *) &threadargs[i]);
        }


        for ( unsigned char i = 0 ; i < num_of_threads ; i++ ) {
                pthread_join(threads[i], NULL); 
        }

        printf("%s", "PRIME\n");
        pthread_exit(NULL);
        return 0;
}
