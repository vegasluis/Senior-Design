#ifndef ADC_H
#define ADC_H

#define intiateADC() 		\
{					 		\
	ADCSRA |= (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);\
	ADMUX |= (1<<REFS0);	\
	ADCSRB = 0x0; 			\
}	

#define startADC0()		\
{				 		\
	DIDR0 = 0X1; 		\
	DDRF  &= (0<<PF0); 	\
	ADMUX = 0x40;		\
}

#define startADC1()		\
{				 		\
	DIDR0 = 0X1; 		\
	DDRF  &= (0<<PF1); 	\
	ADMUX = 0x41;		\
}

#define startADC2()		\
{				 		\
	DIDR0 = 0X2; 		\
	DDRF  &= (0<<PF2); 	\
	ADMUX = 0x42;		\
}

//Wait for conversion to finish 
void waitADC()
{
	ADCSRA |= (1<<ADSC);  // start conversion
	while ((ADCSRA & (1 << ADSC)) == 0); // wait for completion
}



#endif