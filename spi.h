void spi_send(uint8_t spi_data){//SPI送信用関数
  SPDR = spi_data;
  while(!(SPSR&(1<<SPIF)));//転送完了まで待機
  uint8_t dummy = SPDR;
}

uint8_t spi_get(void){//SPI受信用関数
  SPDR = 0x00;
  while(!(SPSR&(1<<SPIF)));//転送完了まで待機
  return SPDR;   
}
