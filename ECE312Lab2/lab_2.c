
//Tazvik Ziauddin
//1659244
//2024-11-01

#define F_CPU 14745600UL

#include "defines.h"
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include "hd44780.h"
#include "lcd.h"

// Setting up the stream for LCD
FILE lcd_str = FDEV_SETUP_STREAM(lcd_putchar, NULL, _FDEV_SETUP_WRITE);

float measuring(void){
    
    PORTB |= (1<<PB2);
    _delay_us(10);
    PORTB &= ~(1<<PB2);
    TIFR1 |= (1<<ICF1);
    TCCR1B |= (1<<ICES1); //Rising edge
    //TIFR1 |= (1<<ICF1);
    
    while(!(TIFR1&(1<<ICF1)));// wait till rising edge
    uint16_t rising_edge_time = ICR1;
    
    
    TCCR1B &= ~(1<<ICES1); //Falling edge
    TIFR1 |= (1<<ICF1);
    while(!(TIFR1&(1<<ICF1)));// wait till falling edge 
    uint16_t falling_edge_time = ICR1;
    
    
    uint16_t time_took = falling_edge_time - rising_edge_time;
    float distance = (time_took)*0.0093045;
    //float distance = (time_took*(1/F_CPU))*(34.3); 
    
    return distance;
    
}

int main(void) {
    
    DDRB &= ~(1<<PB0); //Echo pin as input
    DDRB |= (1<<PB2); //Trigger pin as ouput 
    DDRB &= ~(1<<PB1); // Button as a input
    //DDRC |= (1<<PC4); //Trigger pin as ouput 
    float max;
    float min;
    uint8_t count = 1;
    TCCR1B |= (1<<ICNC1); //noise canceling 
    TCCR1B |= (1<<CS11);
    
    PORTB |= (1<< PB1);//raising the button to a 5V
    //TCCR1B &= ~(1<<CS12);
    lcd_init();
    _delay_ms(50);
    
    //min and max both Nan on start up
    hd44780_outcmd(HD44780_DDADDR(0x41));
    hd44780_wait_ready(false);
    
    fprintf(&lcd_str, "Nan");
    hd44780_wait_ready(false);
    
    hd44780_outcmd(HD44780_DDADDR(0x49));
    hd44780_wait_ready(false);
    
    fprintf(&lcd_str, "Nan");
    hd44780_wait_ready(false);
   
    while(1){ 
        
        
        float dist_measure = measuring();
        
        //check for the button pushed
        if(!(PINB&(1<<PB1))){
            while(!(PINB&(1<<PB1))){
                
            }
            //on the first push make min and max the same which is equal to dist measure
            if (count == 1){
                
                max = dist_measure;
                min = dist_measure;
            
            hd44780_outcmd(HD44780_DDADDR(0x41));
            hd44780_wait_ready(false);
            fprintf(&lcd_str, "%.1fcm", min);
            hd44780_wait_ready(false);
            
            hd44780_outcmd(HD44780_DDADDR(0x49));
            hd44780_wait_ready(false);
    
            fprintf(&lcd_str, "%.1fcm", max);
            hd44780_wait_ready(false);
            count = 2;
                
            }
            //if the distance measured is less than the distance measured before then that will be the new min
            if (dist_measure < min){
            min = dist_measure;
            hd44780_outcmd(HD44780_DDADDR(0x41));
            hd44780_wait_ready(false);
            fprintf(&lcd_str, "%.1fcm", min);
            hd44780_wait_ready(false);
                
           }
            //if the distance measured is greater than the distance measured before then that will be the new max
           else if (dist_measure > max){
              
               max = dist_measure;
               
               hd44780_outcmd(HD44780_DDADDR(0x49));
               hd44780_wait_ready(false);
               fprintf(&lcd_str, "%.1fcm", max);
               hd44780_wait_ready(false);               
           }
            
        }//if the distance measured is greater than 91cm or less than 2cm it is out of range
        else if ( dist_measure > 91 || dist_measure < 2){
     
        hd44780_outcmd(HD44780_DDADDR(0x02));
        hd44780_wait_ready(false);

        fprintf(&lcd_str, "Out of Range");
        hd44780_wait_ready(false);
        
        hd44780_outcmd(HD44780_DDADDR(0x41));
        hd44780_wait_ready(false);
        
        }
        //if the distance measured is less than 91cm or greater than 2cm it is within range so distance measured should be updated
        else if (dist_measure < 91 && dist_measure > 2) {
            
        hd44780_outcmd(HD44780_DDADDR(0x00));
        hd44780_wait_ready(false);

        fprintf(&lcd_str, "                ");
        hd44780_wait_ready(false);
         
        hd44780_outcmd(HD44780_DDADDR(0x04));
        hd44780_wait_ready(false);

        fprintf(&lcd_str, "%.1fcm", dist_measure);
        hd44780_wait_ready(false);
        }

        _delay_ms(1000);
    }
    

    
    return 0;
}