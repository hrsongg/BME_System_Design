#include <msp430x16x.h>

int adc1, adc3, adc4; // Variables to store ADC conversion results
unsigned char Packet[13]; // Data packet for UART transmission
float a1[4] = {1.0000, -0.5772, 0.4218, -0.0563}; // EMG 100Hz Low-Pass Filter coefficients
float b1[4] = { 0.0985, 0.2956, 0.2956, 0.0985}; // EMG 100Hz Low-Pass Filter coefficients
float a2[4] = { 1.0000, -2.4986, 2.1153, -0.6041 }; // EOG 10Hz Low-Pass Filter coefficients
float b2[4] = { 0.0016, 0.0047, 0.0047, 0.0016 }; // EOG 10Hz Low-Pass Filter coefficients

int lpfilt1(int indata1); // Function for EMG 100Hz Low-Pass Filtering
int lpfilt2(int indata2); // Function for EOG Horizontal 10Hz Low-Pass Filtering
int lpfilt3(int indata3); // Function for EOG Vertical 10Hz Low-Pass Filtering

void ReadAdc12(void); // Function to read data from the internal 12-bit ADC

void main(void)
{
    unsigned int i;

    //  Set basic clock and timer
    WDTCTL = WDTPW + WDTHOLD; // Stop the watchdog timer
    BCSCTL1 &= ~XT2OFF; // Enable XT2 oscillator
    do {
        IFG1 &= ~OFIFG; // Clear oscillator flag
        for (i = 0; i < 0xFF; i++); // Delay for oscillator stabilization
    } while ((IFG1 & OFIFG));

    BCSCTL2 |= SELM_2; // Set MCLK to XT2CLK (6MHz)
    BCSCTL2 |= SELS; // Set SMCLK to XT2CLK (6MHz)

    // Configure ports
    P3SEL = BIT4 | BIT5; // Set P3.4 and P3.5 as USART0 TXD/RXD
    P6SEL = 0x01; // Configure P6.0 as analog input
    P6DIR = 0x01; // Set P6.0 as output
    P6OUT = 0x00;   

    // Configure UART0
    ME1 |= UTXE0 + URXE0; // Enable USART0 TXD/RXD
    UCTL0 |= CHAR; // Set character length to 8 bits
    UTCTL0 |= SSEL0 | SSEL1; // Use SMCLK as clock source
    UBR00 = 0x34; // Baud rate: 115200 (6MHz / 52)
    UBR10 = 0x00;                             // 6MHz 115200
    UMCTL0 = 0x00;                            // 6MHz 115200 modulation
    UCTL0 &= ~SWRST; // Initialize USART state machine

    // Configure internal ADC12
    ADC12CTL0 = ADC12ON | REFON | REF2_5V; // Enable ADC and set reference voltage to 2.5V
    ADC12CTL0 |= MSC; // Enable multiple sample and conversion mode
    ADC12CTL1 = ADC12SSEL_3 | ADC12DIV_7 | CONSEQ_1; // Use SMCLK, divide by 8, sequence of channels mode
    ADC12CTL1 |= SHP;

    ADC12MCTL0 = SREF_0 | INCH_0; // Configure channel 0 (EOG Horizontal)
    ADC12MCTL2 = SREF_0 | INCH_2; // Configure channel 2 (EOG Vertical)
    ADC12MCTL3 = SREF_0 | INCH_3 | EOS; // Configure channel 3 (EMG) as end of sequence

    ADC12CTL0 |= ENC; // Enable conversion

    // Configure TimerA
    TACTL = TASSEL_2 + MC_1; // Use SMCLK in up mode
    TACCTL0 = CCIE; // Enable TimerA interrupt
    TACCR0 = 24000; // Set sampling rate to 250Hz (6MHz / 24000)
    _BIS_SR(LPM0_bits + GIE); // Enter low-power mode with interrupts enabled

}


// TimerA interrupt service routine for sampling and data transmission
#pragma vector = TIMERA0_VECTOR
__interrupt void TimerA0_interrupt()
{
    int filtdata1, filtdata2, filtdata3;
    ReadAdc12(); // Read data from the ADC

    Packet[0] = (unsigned char)0x81; // Header byte for data packet
    __no_operation();

    filtdata1 = lpfilt2(adc1) + 5000 - 650 ; // Apply EOG Horizontal filter (10Hz LPF)
    Packet[1] = (unsigned char)(filtdata1 >> 7) & 0x7F; 
    Packet[2] = (unsigned char)filtdata1 & 0x7F;

    filtdata2 = lpfilt3(adc3) + 5000 - 650; // Apply EOG Vertical filter (10Hz LPF)
    Packet[3] = (unsigned char)(filtdata2 >> 7) & 0x7F; 
    Packet[4] = (unsigned char)filtdata2 & 0x7F;

    filtdata3 = lpfilt1(adc4) + 5000 - 650; // Apply EMG filter (100Hz LPF)
    Packet[5] = (unsigned char)(filtdata3 >> 7) & 0x7F; 
    Packet[6] = (unsigned char)filtdata3 & 0x7F;


    unsigned int j;
    for (j = 0; j < 13; j++) {
        while (!(IFG1 & UTXIFG0)); // Wait until UART TX buffer is ready
        TXBUF0 = Packet[j]; // Transmit packet byte by byte via UART
    }
}

// Function to read data from the internal ADC12 and process it into voltage values in mV
void ReadAdc12(void)
{
   /* 
      Convert raw ADC values into voltage values:
      - Scale factor: (4500mV * 2) / resolution (4096)
      - Offset by -4500mV to center around zero (-4500 to +4500mV)
      - Add a constant offset (+700) to avoid losing the sign bit during transmission.
   */
    
    adc1 = (int)((long)ADC12MEM0 * 9000 / 4096) - 4500 + 7000;             // adc0
    adc3 = (int)((long)ADC12MEM2 * 9000 / 4096) - 4500 + 7000;
    adc4 = (int)((long)ADC12MEM3 * 9000 / 4096) - 4500 + 7000;


    ADC12CTL0 |= ADC12SC;                                               // start conversion
    /* ADC=0 : -4.5V, ADC=4095 : 4.5V */
}

int lpfilt1(int indata1)
{
    static float y0=0, y1=0, y2=0, y3=0;
    static int x0=0, x1=0, x2=0, x3=0;
    int output1;

    x0 = indata1;
    y0 = b1[0] * x0 + b1[1] * x1 + b1[2] * x2 + b1[3] * x3 - a1[1] * y1 - a1[2] * y2 -
    a1[3] * y3;
    y3 = y2;
    y2 = y1;
    y1 = y0;
    x3 = x2;
    x2 = x1;
    x1 = x0;
    output1 = (int)y0;
    return(output1);

}

int lpfilt2(int indata2)
{
    static float y4=0, y5=0, y6=0, y7=0;
    static int x4=0, x5=0, x6=0, x7=0;
    int output2;

    x4 = indata2;
    y4 = b2[0] * x4 + b2[1] * x5 + b2[2] * x6 + b2[3] * x7 - a2[1] * y5 - a2[2] * y6 -
    a2[3] * y7;
    y7 = y6;
    y6 = y5;
    y5 = y4;
    x7 = x6;
    x6 = x5;
    x5 = x4;
    output2 = (int)y4;
    return(output2);
}

int lpfilt3(int indata3)
{
    static float y8=0, y9=0, y10=0, y11=0;
    static int x8=0, x9=0, x10=0, x11=0;
    int output3;

    x8 = indata3;
    y8 = b2[0] * x8 + b2[1] * x9 + b2[2] * x10 + b2[3] * x11 - a2[1] * y9 - a2[2] * y10 - a2[3] * y11;
    y11 = y10;
    y10 = y9;
    y9 = y8;
    x11 = x10;
    x10 = x9;
    x9 = x8;
    output3 = (int)y11;
    return(output3);
}