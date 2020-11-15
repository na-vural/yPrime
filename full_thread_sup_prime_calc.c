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
        unsigned short int step = 1;
        unsigned char inc = 2;
        unsigned char mod_result[primes_index];
        unsigned char check;
        unsigned short int i, j;
        for (i = 0 ; i < primes_index ; ) {
                if ( (1 + inc) % primes_array[i] == 0 ) {
                        mod_result[i] = step;
                        step++;
                        inc += 2;
                        i++;
                } else {
                        step++;
                        inc += 2;
                }
        }

        step = 1;
        for (i = 0 ; i < inc_seq_index ; i++) {
                inc = 2;
                check = 0;
                while (check < primes_index) {
                        for (j = 0 ; j < primes_index ; j++) {
                            if (step % primes_array[j] == mod_result[j]) {
                                    step++;
                                    inc += 2;
                                    check = 0;
                            } else {
                                    check++;
                            }
                        }
                }
                inc_seq[i] = inc;
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

        //if ( thread_id % *num_of_threads != 0 ) {
        unsigned char step_init = 0;
        for ( unsigned int i = 0 ; i < thread_id ; i++ ) {
                step_init += inc_seq[i];
        }
        
        mpz_add_ui(divisor, divisor, step_init);
        //}

        unsigned char parallel_inc[inc_seq_index];
        for ( unsigned int i = 0 ; i < inc_seq_index ; i++ ) {
            parallel_inc[i] = 0;
            for ( unsigned char j = 0 ; j < *num_of_threads ; j++ ) {
                parallel_inc[i] 
                +=
                inc_seq[(i * (*num_of_threads) + thread_id + j) % inc_seq_index];
            }
        }

        //gmp_printf("Num = %Zd\nLimit = %Zd\n", *num, *limit);
        //printf("check_byt = %d\nthread_id = %d\nnum_of_threads = %d\n", *check_byt, thread_id, *num_of_threads);
/*
        for (int i ; i < inc_seq_index ; i++) {
                printf("%d ", inc_seq[i]);
        }
*/

        for ( unsigned char i = 0 ; mpz_cmp(*limit, divisor) >= 0 ; i++ ) {
                i %= inc_seq_index;
                if ( mpz_divisible_p(*num, divisor) != 0 ) {
                        gmp_printf("Not prime. Divisible with %Zd", divisor);
                        exit(-1);
                }
                mpz_add_ui(divisor, divisor, parallel_inc[i]);
        }

        return 0;
}



int main(void) 
{
        mpz_t num;
        mpz_init(num);
        printf("%s", "Gimme the integer\n---> ");
        gmp_scanf("%Zd", num);
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
