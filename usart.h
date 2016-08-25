unsigned char rx_usart(void)//受信用関数
{
  while( !(UCSR0A & (1<<RXC0)) );                //受信完了待機
  return UDR0;                                //受信ﾃﾞｰﾀ返す
}

void tx_usart(unsigned char data)//送信用関数
{
  while( !(UCSR0A & (1<<UDRE0)) );        //送信ﾊﾞｯﾌｧ空き待機
  UDR0 = data;
}

void puts_tx(char *str)//文字列送信用関数
{
  while( *str != 0x00 ) //nullになるまで実行
    {
	 tx_usart(*str);
	 str++;                                    //ｱﾄﾞﾚｽ+10    }
    }
}

void tx_line_number(unsigned int val){
  unsigned char a4,a3,a2,a1;//各桁
  a4=(val/1000)+0x30;
  a3=(val/100)%10+0x30;
  a2=(val/10)%10+0x30;
  a1=(val%10)+0x30;

  tx_usart(a4);
  tx_usart(a3);
  tx_usart(a2);
  tx_usart(a1);
  return;
}
