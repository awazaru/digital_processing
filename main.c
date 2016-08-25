/*H28 8/24~
 *AVR digital processing
 *MCU : ATMEGA328P
 *fuse : 
 *20MHz
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "spi.h"

/*AD変換用マクロ*/
#define is_SET(x,y) ((x) & (1<<(y)))
#define smp_f 20000//サンプリング周波数(Hz) ICRの設定値より
#define PICTH_CNT_WIDTH 3
#define PICTH_CNT_MAX 4
/*再生速度=(PITCH_CNT_WIDTH)/(PITCH_CNT_MAX)倍*/

uint8_t	write_address[3]={"0"};	
uint8_t	read_address[3] ={"0"};	
uint8_t adc_delay_width = 0;
uint8_t adc_delay_buf = 0;

/*関数宣言*/
void spi_ini(){//spi通信設定
  //CSはPD2ピン
  SPCR|=_BV(SPE)|_BV(MSTR);
  /*  SPIE    : SPI割り込み許可
	 SPE     : SPI許可(SPI操作を許可するために必須)
	 DORD    : データ順選択,1:LSBから 0:MSBから
	 MSTR    : 1:主装置動作 0:従装置動作
	 CPOL    : SCK極性選択
	 CPHA    :SCK位相選択
	 SPR1,SPR0 : 00:SCK周波数=fosc/4
  */
  /*SPI状態レジスタ SPSR
    SPIF    : SPI割り込み要求フラグ 転送完了時1
    WCOL    :上書き発生フラグ
  */
  /*SPIデータレジスタSPDR
    8bit
    7 6 5 4 3 2 1 0
    (MSB)       (LSB)
  */

  //SPI通信用関数
}

void adc_ini(){
  ADMUX |=_BV(ADLAR);
  /*REFS1 REFS0 : 00 Arefピンの外部基準電圧
    ADPS2 ADPS1 ADPS0 :000 CK/2 ~= 100000Hz(変換クロック)
    MUX3 MUX2 MUX1 MUX0: 0000ADC0ピン
  */
    
  ADCSRA|= _BV(ADEN)|_BV(ADPS1);
  /*ADEN :1 A/D許可
    ADPS2 ADPS1 ADPS0 :010 CK/4 ~= 50000Hz(変換クロック),それ以上だとうまくできない？？
    _BV(ADSC)によって起動 単独変換動
  */
    
  /*ADCSRA|=_BV(ADEN)|_BV(ADATE);
    ADCSRB|=_BV(ADTS2)|_BV(ADTS1)|_BV(ADTS0);
    自動変換タイマ/カウンタ1捕獲*/
  DIDR0 |=_BV(ADC0D)|_BV(ADC1D);
  /*デジタル入力禁止 ADC0,1: PC0,1*/
}

void timer_ini(){//タイマー設定
  /*PWM*/
  TCCR1A |=_BV(COM1A1)|_BV(WGM11);
  /*位相基準PWM OC1A TOP値ICR1*/
  TCCR1B|=_BV(WGM13)|_BV(CS10);
  /*WGM13 WGM12 WGM11 WGM10: 1000 位相基準PWM動作 ICR1
   *CS12 CS11 CS10 : 001 分周なし*/
    
  ICR1 = 499;//割り込み周波数 20000Hz時 499
  //ICR1 = 10000;
  //ICR1_buf = ICR1;
  TIMSK1|=_BV(ICIE1);/*タイマ/カウンタ1捕獲割り込み許可*/
  OCR1B = 0;
}

void pin_ini(){//ピン設定
  DDRB    =   0b00101110;
  /*SCK:1 MISO:0 MOSI:1PB1:OCR1A PB2:CS CSをLOWで選択*/
  PORTB   =   0b00000100;
}


void add_check(uint8_t top, uint8_t middle, uint8_t distingish){//distingish 1ならwrite,0ならread
  uint8_t delay_buf=0;
  if(distingish==1){//wrie
    delay_buf =adc_delay_buf; //0x33*(adc_delay_width/256.0);
  }else {//read
    delay_buf = 0;
  }

  uint16_t add_check=(top<<2)+middle+delay_buf;
  if(add_check >= 0xEFF){
    if(distingish ==1){//writeのaddress
      unsigned char i=0;
      for(i=0;i<=2;i++){
        write_address[i]=0;//i=1:high,2:middle,3:low
      }
    }else{//readのaddress
	 unsigned char j=0;
      for(j=0;j<=2;j++){
        read_address[j]=0;//i=1:high,2:middle,3:low
      }
    }
  }
}

void delay(void){
 
   uint8_t ad_buf=0;
   uint8_t serial_buf=0;
   uint8_t spi_buf=0;


  ADCSRA|= _BV(ADSC);//ADC開始
  while(is_SET(ADCSRA,ADIF)==0);  //変換終了まで待機*/
  ad_buf = ADCH;
  


  //ディレイ用処理(外部SRAM使用)
  add_check(write_address[0],write_address[1],1);//書き込みアドレスの確認
  add_check(read_address[0],read_address[1],0);//読み込みアドレスの確認


  //SRAM操作処理
  PORTB=0b00000000;
  spi_send(0x02);//write
  unsigned char i=0;//配列カウント用 //配列の添字はunsigned char or uint8_t
  for(i=0;i<=2;i++){
  spi_buf=write_address[i];
  spi_send(spi_buf);
  }
  spi_send(ad_buf);
  PORTB=0b00000100;
      
  PORTB=0b00000000;
  spi_send(0x03);//read
  unsigned char j=0;
  for(j=0;j<=2;j++){
  spi_buf=read_address[j];
  spi_send(spi_buf);
  } 
  serial_buf = spi_get();
  PORTB=0b00000100;
  

  //ディレイ時に使う処理
  
  OCR1A = serial_buf;
  
  //delay_sound(buf_ad);


  write_address[2]++;
  if(write_address[2]>=0xFF){
    write_address[2]=0;
    write_address[1]++;
    if(write_address[1]>=0xFF){
	 write_address[1]=0;
	 write_address[0]++;
    }
  }

  read_address[2]++;
  if(read_address[2]>=0xFF){
    read_address[2]=0;
    read_address[1]++;
    if(read_address[1]>=0xFF){
	 read_address[1]=0;
	 read_address[0]++;
    }
  }

}


/*タイマ1 捕獲割り込み*/
ISR(TIMER1_CAPT_vect){
 
  //pitch_shift();
  delay();

}

int main(void){
  //初期化
  spi_ini();
  adc_ini();
  timer_ini();
  pin_ini();

sei();//全体割り込み許可
    
    PORTB = 0b00000000;
    spi_send(0x01);
    spi_send(0x00);
    PORTB = 0b00000100;
    //ADCSRA |=_BV(ADSC);//AD変換初期化兼開始


ADMUX |= _BV(ADLAR);
ADMUX&=~_BV(MUX0);
  while(1){

  }

  return 0;
}
