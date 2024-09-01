#include <LPC17xx.h>
#include <stdlib.h>
#include <stdio.h>
#define RS_CTRL 0x08000000 // P0.27, 1<<27
#define EN_CTRL 0x10000000 // P0.28, 1<<28
#define DT_CTRL 0x07800000 // P0.23 to P0.26 data lines, F<<23
 
unsigned long int temp1 = 0, temp2 = 0, i, j, r, x;
unsigned char flag1 = 0, flag2 = 0, k;
char msg1[16];
char msg2[16];
int count = 0;
int entrf = 0, exitf = 0;
int exted = 0, entred = 0;
void lcd_write(void);
void port_write(void);
void delay_lcd(unsigned int);
unsigned long int init_command[] = {0x30, 0x30, 0x30, 0x20, 0x28, 0x01, 0x06, 0x0c, 0x80};
long j1, x1, x2;
void lcd_write(void)
{
  temp2 = temp1 & 0xf0; // 4 - Bits to get it to least significant digit place
  temp2 = temp2 >> 4;
  port_write();
  if (!((flag1 == 0) && ((temp1 == 0x20) || (temp1 == 0x30)))) // send least significant 4 bits only when it is data/command other than 0x30/0x20
  {
    temp2 = temp1 & 0x0f;
    temp2 = temp2;
    port_write();
  }
}
 
void port_write(void)
 
{
 
  LPC_GPIO0->FIOPIN = temp2 << 23; // sending the ascii code
  if (flag1 == 0)
    LPC_GPIO0->FIOCLR = RS_CTRL; // if command
  else
    LPC_GPIO0->FIOSET = RS_CTRL; // if data
 
  LPC_GPIO0->FIOSET = EN_CTRL; // sending a low high edge on enable input
  for (r = 0; r < 25; r++)
    ;
  LPC_GPIO0->FIOCLR = EN_CTRL;
  for (r = 0; r < 30000; r++)
    ;
}
 
void display()
{
  flag1 = 0;
  for (i = 0; i < 9; i++)
  {
    temp1 = init_command[i];
    lcd_write();
  }
  flag1 = 1; // DATA MODE
  for (i = 0; msg1[i] != '\0'; i++)
  {
    temp1 = msg1[i];
    lcd_write();
  }
  flag1 = 0;
  temp1 = 0xC0;
  lcd_write();
  flag1 = 1;
  sprintf(msg2, "%d", count);
  for (i = 0; msg2[i] != '\0'; i++)
  {
    temp1 = msg2[i];
    lcd_write();
  }
}
 
int main(void)
{
  SystemInit();
  SystemCoreClockUpdate();
  sprintf(msg1, "Headcount is:");
  LPC_PINCON->PINSEL3 = 0; // SENSOR
  LPC_PINCON->PINSEL0 = 0; // LED
  LPC_GPIO1->FIODIR = 0;
 
  // LCD
  LPC_PINCON->PINSEL1 = 0;
  LPC_PINCON->PINSEL4 = 0;
  LPC_GPIO0->FIODIR = DT_CTRL | RS_CTRL | EN_CTRL | (0xFF << 4); // 0xf<<23 | 1<<27 | 1<<28;
  LPC_GPIO2->FIODIR = 0 << 12;
 
  display();
 
  while (1)
  {
    // LPC_GPIO0->FIOCLR=0xFF<<4;
    x1 = LPC_GPIO1->FIOPIN & 1 << 23;
    x2 = LPC_GPIO1->FIOPIN & 1 << 24;
    if (x1)
    {
 
      if (x2)
      {
        // no entry no exit
        entrf = 0;
        exitf = 0;
      }
      else
      {
        // exit
        entrf = 0;
        if (exitf == 1)
          continue;
        exitf = 1;
        if (entred == 1)
        {
          entred = 0;
          count++;
          LPC_GPIO0->FIOSET = 0xF << 8;
          display();
        }
        else
        {
          exted = 1;
        }
      }
    }
    else
    {
      // entry
      if (!x2)
        continue;
      exitf = 0;
      if (entrf == 1)
        continue;
      entrf = 1;
      if (exted == 1)
      {
        exted = 0;
        count--;
        if (count < 0)
          count = 0;
        LPC_GPIO0->FIOSET = 0xF << 4;
        display();
      }
      else
      {
        entred = 1;
      }
    }
    if(count > 0){
      LPC_GPIO0->FIOSET |= 0xff<<8;
    }
    else{
      LPC_GPIO0->FIOCLR |= 0xff<<8;
    }
    for (j1 = 0; j1 < 100000; j1++)
      ;
  }
}