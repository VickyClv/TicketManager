# TicketManager

A system for purchusing seats at a theater, using the POSIX threads (pthreads) package.

-----

In this system, each customer initially books the seats that he wants, then pays for them by credit card and finally the money is transferred to the company's account. In these systems we have a large number customers served by a limited number of service points, therefore the program implements mutual blocking (with mutexes) and synchronization (with condition variables).

-----

Objective: The seat booking system has a bank account, a call center with Ntel operators who serve customers (find seats) and Ncash cashiers who complete the transaction (charge the credit cards). The theater seats have a rectangular arrangement, with each row having Nseat seats and the rows being divided into two zones: the front NzoneA rows cost CzoneA euros per seat and the rear NzoneB rows cost CzoneB euros per seat. The first customer calls at time 0, and each subsequent customer calls after a random integer number of seconds in the interval [treslow, treshigh]. When all answering telephone operators are busy, the customer waits for the next available operator. When a customer connects to an operator, it randomly selects a zone with probability PzoneA for the front rows or (1-PzoneA) for the back rows, and a random integer number of tickets in interval [Nseatlow, Nseathigh]. The operator takes a random integer number of seconds in the interval [tseatlow,tseathigh] to test if there are enough consecutive seats in any of the rows in that zone. If there are no consecutive seats, the customer receives an error message and ends the call. If there are contiguous positions, they are reserved in plan of the theater, their total cost is calculated, and the customer proceeds to pay with the credit card. When all cashiers are busy, the customer waits for the next cashier available. When the customer connects to the cashier, the cashier needs a random integer number of seconds in the interval [tcashlow,tcashhigh] to try to make the payment. With probability Pcardsuccess the payment is accepted, the customer is charged the appropriate cost, the money is transferred to the company's account and the customer is informed of the total cost and the seat numbers in the selected zone. If payment fails, the reserved seats become available again for booking by the cashier and the customer gets an error message and completes the call. When Ncust clients are processed, the system prints its results.

-----

Input and data: The initial amount in the company account is 0 euros. The following constants are defined in a declaration file:

• Ntel=3 telephone operators, Ncash=2 cashiers

• Nseat=10 seats per row

• NzoneA=10 rows, NzoneB=20 rows

• PzoneA=30%• CzoneA=30 euros, CzoneB=20 euros

• Nseatlow=1 seat, Nseathigh=5 seats

• treslow=1 sec, treshigh=5 sec

• tseatlow=5 sec, tseathigh=13 sec

• tcashlow=4 sec, tcashhigh=8 sec

• Pcardsuccess=90%

The program accepts two parameters from the command line: the number of customers to serve, Ncust, and the random seed for the random number generator.

-----

Output: For each customer one of the following messages will be printed on the screen, depending on how the call ended, which will start with the customer number:

• Client <Client_ID>:The reservation was completed succesfully. Your seats are in Zone <selected_zone>, Row <selected_row>, Seat <selected_seats> and the final cost is <cost> euros.
  
• The order for client <Client_ID> failed due to shortage in seats.
  
• The order for client <Client_ID> failed due to the credit card not being accepted.
  
Finally, the system will print the following:
  
• The seating plan, e.g. Zone A / Row 1 / Seat 1 / Customer 3, Zone 2 / Row 5 / Seat 2 / Customer 4, etc.
  
• Total revenue from sales.
  
• The percentage of transactions that are completed with each of the above three ways.
  
• The average waiting time of customers (from the moment the customer appears until speak to the operator and, if the transaction proceeds to payment, from the moment that the telephone operator is done until the time the cashier takes over).
  
• The average customer service time (from the moment the customer appears, to the completion or failure of the booking).

  
  
Code structure: The initial thread of the program will spawn one thread per client (total Ncust threads). Each thread will then perform the above steps until its order is complete and it will print the appropriate output. Finally, the original thread will print the final output. 
  
  
