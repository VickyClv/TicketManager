#include <pthread.h> 
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "theater-res.h"

int avail_tel = 3;
pthread_mutex_t lock_tel; 
pthread_cond_t cond_tel;

int avail_cash = 2;
pthread_mutex_t lock_cash;
pthread_cond_t cond_cash;

int* seats[30];
pthread_mutex_t lock_array;

int final_revenue = 0;
pthread_mutex_t lock_bank;

int counter_success = 0;
int counter_fail_seats = 0;
int counter_fail_credit = 0;
pthread_mutex_t lock_statistics;

long int sum_wait_tel = 0; 
int sum_tel_cash = 0; 
int sum_complete = 0;
pthread_mutex_t lock_wait_tel, lock_tel_cash, lock_complete;

static int seed_tr;


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Main

int main(int argc, char *argv[])  {

	if (argc != 3) {
		printf("ERROR: the program should take two arguments, the number of clients and the seed!\n");
		exit(-1);
	}


	// Initialization - input
	int num_of_threads = atoi(argv[1]);
	if (num_of_threads < 0) {
		printf("ERROR: the number of threads to run should be a positive number. Current number given %d.\n", num_of_threads);
		exit(-1);
	}

	pthread_t threads[num_of_threads];
	int id[num_of_threads];

	int rc;

	seed_tr = atoi(argv[2]);
	unsigned int seedp = seed_tr;
	
	sum_wait_tel = 0.0;
	sum_tel_cash = 0.0;
	sum_complete = 0.0;


	// Initialization - array
	for (int i = 0; i < 30; i++) {
		seats[i] = (int*) malloc(sizeof(int) * 11);
	}
	for (int i = 0; i < NzoneA + NzoneB; i++) {
		for (int j = 0; j < Nseat; j++) {
			seats[i][j] = 0;
		}
		seats[i][10] = 10;
	}


	// Initialization - mutexes & conditions
	pthread_mutex_init(&lock_tel, NULL);
	pthread_cond_init(&cond_tel, NULL);
	pthread_mutex_init(&lock_cash, NULL);
	pthread_cond_init(&cond_cash, NULL);
	pthread_mutex_init(&lock_array, NULL);
	pthread_mutex_init(&lock_bank, NULL);
	pthread_mutex_init(&lock_statistics, NULL);
	pthread_mutex_init(&lock_wait_tel, NULL);
	pthread_mutex_init(&lock_tel_cash, NULL);
	pthread_mutex_init(&lock_complete, NULL);


	// Create Threads
	for (int i = 0; i < num_of_threads; i++) {
		id[i] = i+1;
		rc = pthread_create(&threads[i], NULL, theater, &id[i]);
		if (rc != 0) {
			printf("ERROR: return code from pthread_create() is %d\n", rc);
       		exit(-1);
       	}
		if (id[i] < num_of_threads - 1) {
			unsigned int rand_call = (rand_r(&seedp) % (treshigh - treslow + 1)) + treslow; //int num = (rand() % (upper - lower + 1)) + lower;
			sleep(rand_call);
		}
	}
	
	
	// Terminate Threads
	for (int i = 0; i < num_of_threads; i++) {
		rc = pthread_join(threads[i], NULL);
		if (rc != 0) {
			printf("ERROR: return code from pthread_join() is %d\n", rc);
			exit(-1);		
		}
	}

	
	// Print results
	printf("\n");
	printf("The seat arrangement is: \n");
	for (int i = 0; i < NzoneA + NzoneB; i++) {
		for (int j = 0; j < Nseat; j++) {
			if (seats[i][j] != 0) {
				if (i < 10) {
					printf("Zone A %c Row %d %c Seat %d %c Client %d \n", 92, i + 1, 92, j + 1, 92, seats[i][j]);
				}
				else {
					printf("Zone B %c Row %d %c Seat %d %c Client %d \n", 92, i - NzoneA + 1, 92, j + 1, 92, seats[i][j]);
				}
			}
		}
	}
	printf("\n");

	// Revenue - Rates
	printf("The total revenue from the sales is %d. \n", final_revenue);

	printf("The number of clients that completed the transaction successfully consist the %d%% of clients.\n", counter_success);
	printf("The number of clients that failed to complete the transaction, because there were no more seats available consist the %d%% of clients.\n", counter_fail_seats);
	printf("The number of clients that failed to complete the transaction, because their credit card wasn't accepted consist the %d%% of clients.\n", counter_fail_credit);
	
	// Average time
	printf("The average time a client had to wait until the telephonist answered was %lf sec.\n", sum_wait_tel / (double)num_of_threads);
	printf("The average time a client had to wait from the time he stopped talking to the telephonist until the time the cashier took over is %lf sec.\n", (double)sum_tel_cash / (double)(num_of_threads - counter_fail_seats));
	printf("The average customer service time was %lf sec.\n", sum_complete / ((double)num_of_threads));


	// Free seats array
	for (int i = 0; i < 30; i++) {
		free(seats[i]);
	}


	// Destruction - mutexes & conditions
	pthread_mutex_destroy(&lock_tel);
	pthread_cond_destroy(&cond_tel);
	pthread_mutex_destroy(&lock_cash);
	pthread_cond_destroy(&cond_cash);
	pthread_mutex_destroy(&lock_array);
	pthread_mutex_destroy(&lock_bank);
	pthread_mutex_destroy(&lock_statistics);
	pthread_mutex_destroy(&lock_wait_tel);
	pthread_mutex_destroy(&lock_tel_cash);
	pthread_mutex_destroy(&lock_complete);


	// End
	return 0;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Theater

void *theater(void *x) {

	// Initialization - arguments
	int id = *(int *)x;

	unsigned int seedp = seed_tr + id;
	int rc;
	int calc_time1; 
	int calc_time2;
	int calc_time3;
	char zone;
	int num_tickets;
	int first_seat;
	int row;
	int payment;

	struct timespec init_time, tel_start_time, tel_end_time, cash_start_time, cl_end_time;


	// Time: Client calls
	clock_gettime(CLOCK_REALTIME, &init_time);


	//TELEPHONIST


	// Client connects to telephonist
	rc = pthread_mutex_lock(&lock_tel);
	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
		exit(-1);		
	}
	while (avail_tel == 0) {
		rc = pthread_cond_wait(&cond_tel, &lock_tel);
		if (rc != 0) {
			printf("ERROR: return code from pthread_cond_wait() is %d\n", rc);
			exit(-1);		
		}
	}
    	avail_tel--;


	// Time: Client connection to telephonist
	clock_gettime(CLOCK_REALTIME, &tel_start_time);


	// Statistics: Time: Client calls -> Client connects to telephonist
	rc = pthread_mutex_lock(&lock_wait_tel);
	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
		exit(-1);		
	}
	calc_time1 = (int) (tel_start_time.tv_sec - init_time.tv_sec);
	sum_wait_tel += (long int) calc_time1;
	rc = pthread_mutex_unlock(&lock_wait_tel);
	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		exit(-1);		
	}


	// Choose zone
	unsigned int randZone = (rand_r(&seedp) % (100 + 1));
	if (randZone <= PzoneA) {
		zone = 'A';
	}
	else {
		zone = 'B';
	}


	// Choose number of tickets
	unsigned int randTickets = (rand_r(&seedp) % (Nseathigh - Nseatlow + 1) + Nseatlow);


	// Sleep: Telephonist searching for seats
	unsigned int randTimeSeats = (rand_r(&seedp) % (tseathigh - tseatlow + 1) + tseatlow);
	sleep(randTimeSeats);


	// Search for seats
	rc = pthread_mutex_lock(&lock_array);
	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
		exit(-1);		
	}
	int found = 0;
	if (zone == 'A') {
		for (int i = 0; i < NzoneA; i++) {
			if (seats[i][10] >= randTickets && found == 0) {
				for (int j = 0; j < Nseat; j++) {
					if (seats[i][j] == 0 && found == 0) {
						row = i;
						first_seat = j;
						found = 1;
						seats[i][10] -= randTickets;
						for(int k = 0; k < randTickets; k++) {
							seats[i][j+k] = id;
							
						}
						break;
					}
				}
			}
		}
	}
	else {
		for (int i = NzoneA; i < NzoneB; i++) {
			if (seats[i][10] >= randTickets && found == 0) {
				for (int j = 0; j < Nseat; j++) {
					if (seats[i][j] == 0 && found == 0) {
						row = i;
						first_seat = j;
						found = 1;
						seats[i][10] -= randTickets;
						for(int k = 0; k < randTickets; k++) {
							seats[i][j+k] = id;	
						}
						break;
					}
				}
			}
		}
	}
	rc = pthread_mutex_unlock(&lock_array);
	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		exit(-1);		
	}


	// FIN: Seats not found
	if (found == 0) {
		rc = pthread_mutex_lock(&lock_statistics);
		if (rc != 0) {
			printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
			exit(-1);		
		}
		printf("The order for client %d failed due to shortage in seats. \n", id);
		counter_fail_seats++;
        	rc = pthread_mutex_unlock(&lock_statistics);
        	if (rc != 0) {
			printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
			exit(-1);		
		}

		// Release telephonist
		avail_tel++;
		rc = pthread_cond_signal(&cond_tel);
		if (rc != 0) {
			printf("ERROR: return code from pthread_cond_signal() is %d\n", rc);
			exit(-1);		
	    	}
	 	rc = pthread_mutex_unlock(&lock_tel);
         	if (rc != 0) {
		    	printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		    	exit(-1);		
	    	}


        	// Statistics: Time: Client calls -> Client disconnects
        	rc = pthread_mutex_lock(&lock_complete);
       	 if (rc != 0) {
		    	printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
		    	exit(-1);		
	    	}
        	calc_time3 = (int) (cl_end_time.tv_sec - init_time.tv_sec); 
        	sum_wait_tel += calc_time3; 
        	rc = pthread_mutex_unlock(&lock_complete);
        	if (rc != 0) {
		   	 printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		   	 exit(-1);		
	    	}
	 	
		pthread_exit(NULL);
	}


    	// Cost of tickets
    	if (zone == 'A') {
		payment = randTickets * CzoneA;
	}
	else {
		payment = randTickets * CzoneB;
	}


    	// Release telephonist
    	avail_tel++;
    	rc = pthread_cond_signal(&cond_tel);
    	if (rc != 0) {
		printf("ERROR: return code from pthread_cond_signal() is %d\n", rc);
		exit(-1);		
	}
	rc = pthread_mutex_unlock(&lock_tel);
    	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		exit(-1);		
	}


    	// Time: Disonnection from telephonist
	clock_gettime(CLOCK_REALTIME, &tel_end_time);


    	//************************************************************************************************************************
	
    	//CASHIER

    	//connect to cashier
    	rc = pthread_mutex_lock(&lock_cash);
    	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
		exit(-1);		
	}
    	while (avail_cash == 0) {
		rc = pthread_cond_wait(&cond_cash, &lock_cash);
        	if (rc != 0) {
		   	 printf("ERROR: return code from pthread_cond_wait() is %d\n", rc);
		    	exit(-1);		
	    	}
	}
    	avail_cash--;

    	// Time: Connect to cashier
    	clock_gettime(CLOCK_REALTIME, &cash_start_time);


    	// Statistics: Time: Client disconnects from telephonist -> Client connects to cashier
    	rc = pthread_mutex_lock(&lock_tel_cash);
    	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
		exit(-1);		
	}
    	calc_time2 = (int) (cash_start_time.tv_sec - tel_end_time.tv_sec);
    	sum_tel_cash += calc_time2;
    	rc = pthread_mutex_unlock(&lock_tel_cash);
   	 if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		exit(-1);		
	}


    	// Sleep: Cashier trying to complete the payment
    	unsigned int randTimeCredit = (rand_r(&seedp) % (tcashhigh - tcashlow + 1) + tcashlow);
	sleep(randTimeCredit);


    	// Completion or failure of transaction
    	unsigned int randSuccess = (rand_r(&seedp) % (100 + 1));
	if (randSuccess <= Pcardsuccess) {

        	// Add the payment to theater's account
		rc = pthread_mutex_lock(&lock_bank);
        	if (rc != 0) {
			printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
			exit(-1);		
		}
		final_revenue += payment;
		rc = pthread_mutex_unlock(&lock_bank);
        	if (rc != 0) {
			printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
			exit(-1);		
	    	}

        	// Create a string with the seats the client bought
		char *str = (char *) malloc(sizeof(char) * 20);
		char *temp_str = (char *) malloc(sizeof(char) * 10);
		strncpy(str, "", sizeof(str));
		if (zone == 'A') {
			for (int i = 0; i < randTickets; i++) {
				sprintf(temp_str, "%d", first_seat + i + 1);
				strncat(str, temp_str, 2);
				if (i < randTickets - 1) {
					strncat(str, ", ", 3);
 				}
			}
		}
		else {
			for (int i = 0; i < randTickets; i++) {
				sprintf(temp_str, "%d", first_seat + i + 1);
				strncat(str, temp_str, 2);
 				if (i < randTickets - 1) {
					strncat(str, ", ", 3);
				}
			}
		}
		free(temp_str);
		

        	// FIN: Transaction Completed
        	rc = pthread_mutex_lock(&lock_statistics);
        	if (rc != 0) {
			printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
			exit(-1);		
		}
		if (zone == 'A') {
			printf("Client %d:The reservation was completed succesfully. Your seats are in Zone %c, Row %d, Seat %s and the final cost is %d euros. \n", id, zone, row + 1, str, payment);
		}
		else {
			printf("Client %d:The reservation was completed succesfully. Your seats are in Zone %c, Row %d, Seat %s and the final cost is %d euros. \n", id, zone, row - NzoneA + 1, str, payment);
		}
		free(str);
		counter_success++;
		rc = pthread_mutex_unlock(&lock_statistics);
		if (rc != 0) {
			printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
			exit(-1);		
		}	
	}
	else {
        	// FIN: Transaction Failed
		rc = pthread_mutex_lock(&lock_statistics);
		if (rc != 0) {
			printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
			exit(-1);		
		}
		printf("The order for client %d failed due to the credit card not being accepted. \n", id);
		counter_fail_credit++;
		rc = pthread_mutex_unlock(&lock_statistics);
		if (rc != 0) {
			printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
			exit(-1);		
		}


		// Return the seats
		rc = pthread_mutex_lock(&lock_array);
		if (rc != 0) {
			printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
			exit(-1);		
		}
		if (zone = 'A') {
			for (int j = first_seat; j < first_seat + randTickets; j++) {
				seats[row][j] = 0;
			}
		}
		else {
			for (int j = first_seat; j < first_seat + randTickets; j++) {
				seats[NzoneA + row][j] = 0;
 			}
		}
        	rc = pthread_mutex_unlock(&lock_array);
        	if (rc != 0) {
		    	printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		    	exit(-1);		
	    	}
	}


	// Release cashier
    	avail_cash++;
	rc = pthread_cond_signal(&cond_cash);
    	if (rc != 0) {
		printf("ERROR: return code from pthread_cond_signal() is %d\n", rc);
		exit(-1);		
	}
	rc = pthread_mutex_unlock(&lock_cash);
    	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		exit(-1);		
	}


    	// Time: End of connection
    	clock_gettime(CLOCK_REALTIME, &cl_end_time);


    	// Statistics: Time: Client calls -> Client disconnects
    	rc = pthread_mutex_lock(&lock_complete);
    	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
		exit(-1);		
	}
    	calc_time3 = (int) (cl_end_time.tv_sec - init_time.tv_sec); 
    	sum_complete += calc_time3; 
    	rc = pthread_mutex_unlock(&lock_complete);
    	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		exit(-1);	
	}
	
    
    	//End of theater()
    	pthread_exit(NULL);
}
