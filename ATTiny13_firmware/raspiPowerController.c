
// RaspiPowerController circuit firmware
//
// MCU: ATTiny13 
// Author: Salvatore Carotenuto of Startup Solutions / OpenMakersItaly
// (mailto: carotenuto@startupsolutions.it)
//
//
// Changelog:
//    2014-12-05 - started writing
//
// ---------------------------------------------------------------------


#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

// ===== PROGRAM DEFINITIONS ===========================================

#define STATUS_STANDBY		1
#define STATUS_POWERON		2
#define STATUS_SHUTDOWN		3

#define BUTTON_PIN			0 	// PB0
#define RASPI_STATUS_PIN	1 	// PB1
#define RASPI_SHUTDOWN_PIN	2 	// PB2
#define POWER_PIN			3 	// PB3
#define LED_PIN				4 	// PB4

// note: with clock=4.8MHz, CLKDIV=y and prescaler=clk/64 we should get ~35 overflows per second
#define OVERFLOWS_PER_SECOND			35

// =====================================================================


// global variables
volatile uint8_t  timerTicks;
volatile uint8_t  sleepTimer;
uint8_t  status = 0;


// ---------------------------------------------------------------------
// ---------------------------------------------------------------------
// ---------------------------------------------------------------------

// function prototypes
void wait(uint16_t periods);
void init(void);
void setPower(uint8_t status);
void setLed(uint8_t status);

// ---------------------------------------------------------------------
// ---------------------------------------------------------------------
// ---------------------------------------------------------------------


// Timer0 overflow interrupt handler
ISR(TIM0_OVF_vect) 
	{ 
	if(timerTicks == OVERFLOWS_PER_SECOND)
		timerTicks = 0;
	else
		timerTicks++;
	//
	if(sleepTimer > 0)
		sleepTimer--;
	}


// ---------------------------------------------------------------------


// suspends execution for a specified period of time
// 1 period: 1/35th of second
void wait(uint16_t periods)
	{
	sleepTimer = periods;
	while(sleepTimer != 0);
	}
	
	
// ---------------------------------------------------------------------


// device initializations
void init()
	{
	// resets timer0 value 	
	TCNT0 = 0x00; 

	// Clock source & prescaling select (clk/64)
	TCCR0B &= ~(1<<CS02); 
	TCCR0B |= (1<<CS01);       
	TCCR0B |= (1<<CS00);

	// WGM2:0 = 0b000: normal mode of operation 
	TCCR0B &= ~(1<<WGM02);             
	TCCR0A &= ~((1<<WGM00) | (1<<WGM01));    

	// Enables interrupt from timer 0 overflow 
	TIMSK0 |= 1<<TOIE0;
	
	DDRB = 0x00;
	
	// sets LED_PIN as output and low
	DDRB |= (1 << LED_PIN);
	PORTB &= ~(1 << LED_PIN);

	// sets POWER_PIN as output and high (circuit uses a P-Channel MOSFET)
	DDRB |= (1 << POWER_PIN);
	PORTB |= (1 << POWER_PIN);

	// sets BUTTON_PIN as input, and enables pullup on it
	DDRB &= ~(1 << BUTTON_PIN);
	PORTB |= (1 << BUTTON_PIN);

	// sets RASPI_STATUS_PIN as input 
	DDRB &= ~(1 << RASPI_STATUS_PIN);
	PORTB &= ~(1 << RASPI_STATUS_PIN);

	// sets RASPI_SHUTDOWN_PIN as output
	DDRB |= (1 << RASPI_SHUTDOWN_PIN);
	PORTB &= ~(1 << RASPI_SHUTDOWN_PIN);

	// default status
	status = STATUS_STANDBY;
	
	// reset counters
	timerTicks = 0;
	sleepTimer = 0;
	}	
	

// ---------------------------------------------------------------------


void setPower(uint8_t status)
	{
	if(status)
		// sets POWER_PIN as low
		PORTB &= ~(1 << POWER_PIN);
	else
		// sets POWER_PIN as high
		PORTB |= (1 << POWER_PIN);
	}

	
// ---------------------------------------------------------------------


void setLed(uint8_t status)
	{
	switch(status)
		{
		case 0:
			// sets LED_PIN as low
			PORTB &= ~(1 << LED_PIN);
			break;
		case 1:
			// sets LED_PIN as high
			PORTB |= (1 << LED_PIN);
			break;
		case 2:
			// toggles LED_PIN status
			PORTB ^= (1 << LED_PIN);
			break;
		}
	}

	
// ---------------------------------------------------------------------


int main (void)
	{
	// disables interrupts
	cli();
	
	// initializes device
	init();
	
	// enables interrupts
	sei();
	
	for(;;) 
		{ 
		switch(status)
			{
			case STATUS_STANDBY:
				// checks status of BUTTON_PIN
				// if button pressed (pin low) sets
				// power and led pins as high,
				// and goes in POWERON status
				if(!(PINB & (1 << BUTTON_PIN)))
					{
					setPower(1);
					setLed(1);
					status = STATUS_POWERON;
					//
					// waits for at least 3 seconds
					wait(150);
					}
				break;
			//
			//
			case STATUS_POWERON:
				// if RASPI_STATUS_PIN is high (raspberry pi up and running)
				if(PINB & (1 << RASPI_STATUS_PIN))
					{
					// checks status of button pin
					// if button pressed (pin low) goes in SHUTDOWN status
					if(!(PINB & (1 << BUTTON_PIN)))
						{
						status = STATUS_SHUTDOWN;
						// sets shutdown pin as high
						PORTB |= (1 << RASPI_SHUTDOWN_PIN);
						}
					}
				break;
			//
			//
			case STATUS_SHUTDOWN:
				// toggles status of led
				setLed(2);
				// checks status of RASPI_STATUS_PIN pin
				// if pin is low, the raspberry pi has finished shutdown
				if(!(PINB & (1 << RASPI_STATUS_PIN)))
					{
					// turns off raspberry pi power and led, and sets shutdown pin as low
					setPower(0);
					setLed(0);
					PORTB &= ~(1 << RASPI_SHUTDOWN_PIN);
					// and goes back in standby mode
					status = STATUS_STANDBY;
					}
				else
					wait(5);
				break;
			}
		wait(2);
		} 
    
	return 0; 
	}
