#ifndef BUTTONS_H
#define BUTTONS_H

// pin pin[5:4]=port number (0, 1, 2), pin[3:0]=pin number (0 to 15)
#define BUTTON1  (0x1E)     // GPIO1_14
#define BUTTON2  (0x0B)     // GPIO0_11
#define BUTTON3  (0x0C)     // GPIO0_12
#define BUTTON4  (0x0D)     // GPIO0_13
#define BUTTON5  (0x20)     // GPIO2_0
#define BUTTON6  (0x21)     // GPIO2_1
#define BUTTON7  (0x22)     // GPIO2_2
#define BUTTON8  (0x23)     // GPIO2_3
#define BUTTON9  (0x24)     // GPIO2_4

#define BUTTON10 (0x1A)     // GPIO1_10
#define BUTTON11 (0x1B)     // GPIO1_11
#define BUTTON12 (0x1C)     // GPIO1_12
#define BUTTON13 (0x1D)     // GPIO1_13

#define ACTIVEHIGH (1)
#define ACTIVELOW  (0)

#define UP       (1)
#define DOWN     (0)



#endif // BUTTON_H