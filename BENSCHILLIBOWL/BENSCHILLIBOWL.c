
#include "BENSCHILLIBOWL.h"

#include <assert.h>
#include <stdlib.h>
#include <time.h>

void AddOrderToBack(Order **orders, Order *order);

MenuItem BENSCHILLIBOWLMenu[] = { 
    "BensChilli", 
    "BensHalfSmoke", 
    "BensHotDog", 
    "BensChilliCheeseFries", 
    "BensShake",
    "BensHotCakes",
    "BensCake",
    "BensHamburger",
    "BensVeggieBurger",
    "BensOnionRings",
};
int BENSCHILLIBOWLMenuLength = 10;

/* Select a random item from the Menu and return it */
MenuItem PickRandomMenuItem() {
    return BENSCHILLIBOWLMenu[rand() % BENSCHILLIBOWLMenuLength];
}

/* Allocate memory for the Restaurant, then create the mutex and condition variables needed to instantiate the Restaurant */

BENSCHILLIBOWL* OpenRestaurant(int max_size, int expected_num_orders) {
    BENSCHILLIBOWL *bcb = (BENSCHILLIBOWL*) malloc(sizeof(BENSCHILLIBOWL));
    bcb->orders = NULL;
    bcb->current_size = 0;
	  bcb->next_order_number=1;
    bcb->orders_handled=0;
    bcb->max_size = max_size;
    bcb->expected_num_orders=expected_num_orders;
  
    pthread_mutex_init(&(bcb->mutex), NULL);  // initiate the mutex
    printf("Restaurant is open!\n");
    return bcb;
}


/* check that the number of orders received is equal to the number handled (ie.fullfilled). Remember to deallocate your resources */

void CloseRestaurant(BENSCHILLIBOWL* mcg) {
    if (mcg->orders_handled != mcg->expected_num_orders) {    // ensure all the orders were handled
        fprintf(stderr, "Not all the orders were handled.\n");
        exit(0);
    }
    pthread_mutex_destroy(&(mcg->mutex));  // delete the synchronization variables
    free(mcg);   // free space used by restaurant
    printf("Restaurant is closed!\n");
}

/* add an order to the back of queue */
int AddOrder(BENSCHILLIBOWL* mcg, Order* order) {
    pthread_mutex_lock(&(mcg->mutex)); //order queue is in critical section. grab the lock
  
    while (mcg->current_size == mcg->max_size) { // wait until restaurant is not full
        pthread_cond_wait(&(mcg->can_add_orders), &(mcg->mutex));
    }
  
    order->order_number = mcg->next_order_number;
    AddOrderToBack(&(mcg->orders), order);
  
    mcg->next_order_number++; //update next order number 
    mcg->current_size++;   //update current size
    
    pthread_cond_broadcast(&(mcg->can_get_orders)); // sending signal to indicate an order had been added.
        
    pthread_mutex_unlock(&(mcg->mutex)); // sending signal to indicate an order had been added.
    
    return order->order_number;
}

/* get an order from the front of the queue */
Order *GetOrder(BENSCHILLIBOWL* mcg) {
    pthread_mutex_lock(&(mcg->mutex)); // grabbing the lock as order is in critical section
       
    while(mcg->current_size == 0) {  // waiting until restaurant is not empty
        
        // If all the orders have been already fulfilled notify cook by unlocking the mutex.
        if (mcg->orders_handled >= mcg->expected_num_orders) {
            pthread_mutex_unlock(&(mcg->mutex));
            return NULL;
        }
        pthread_cond_wait(&(mcg->can_get_orders), &(mcg->mutex));
    }
    
    // Get the order from the front.
    Order *order = mcg->orders;
    mcg->orders = mcg->orders->next;
    
    mcg->current_size--; // update the current order size
    mcg->orders_handled++; // update the orders handled
    
    pthread_cond_broadcast(&(mcg->can_add_orders));
        
    // Release the lock.
    pthread_mutex_unlock(&(mcg->mutex));
    
    
    return order;

}


/* this methods adds order to rear of queue */
void AddOrderToBack(Order **orders, Order *order) {
  if (*orders == NULL) { // If there were no orders.
        *orders = order;
    } else {  // If orders isn't empty.
        Order *curr_order = *orders;
        while (curr_order->next) {
            curr_order = curr_order->next;
        }
        curr_order->next = order;
    }
}