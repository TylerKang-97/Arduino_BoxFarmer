#include "DHT.h" //DHT 라이브러리 호출
//#include <DigitShield.h> //DigitShield.h 라이브러리 호출
#include <Servo.h> //서보모터 라이브러리 호출
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#define DHTPIN 12     // 온습도 센서가 12번에 연결
#define DHTTYPE DHT11   // DHT11 온습도 센서 사용
#define SensorPin A0     //pH 센서 A0번에 연결
#define Offset 0.00     //deviation compensate
#define printInterval 800
#define ArrayLenth  40
#define samplingInterval 20

LiquidCrystal_I2C lcd(0x27, 20, 4);
int pHArray[ArrayLenth];   //Store the average value of the sensor feedback
int pHArrayIndex = 0;

DHT dht(DHTPIN, DHTTYPE); //DHT 설정(12,DHT11)
Servo microServo;

int btnvalueup = 7;
int btnvaluedown = 6;
int btnposleft = 5;
int btnposright = 3;
int btnyes = 22;

int motor = 10; //모터 릴레이 핀 10번
int water = 0 ; // water 변수 선언
int hitter = 11; //스페이스 히터 핀 11번
int fan = 9; // 쿨링팬 핀 9번
int servoPin = 2; //서보모터 핀 2번
int angle = 0; //서보모터 각도 변수

//값 조정을 위한 변수들--------------------------------------------------------------------------------------------------------------------------------------------------------------------
int watervalue = 800; //물주가 변수
int hitvalue = 20; //물주기 온도변수
int fanhit, fanh; //팬온도, 팬습도
int phValue = 5;

int pos[5] = {800, 20, 50, 80, 5}; //배열
int p;
int nowtime = 1;
int lasttime = 0;
bool change = true;

void setup()
{
  Serial.begin(9600); //통신속도 9600으로 통신 시작
  // DigitShield.begin();
  dht.begin();
  Serial.begin(9600);
  pinMode(motor, OUTPUT);
  pinMode(hitter, OUTPUT);
  pinMode(fan, OUTPUT);
  pinMode(btnvalueup, INPUT_PULLUP);
  pinMode(btnvaluedown, INPUT_PULLUP);
  pinMode(btnposleft, INPUT_PULLUP);
  pinMode(btnposright, INPUT_PULLUP);
  pinMode(btnyes, INPUT_PULLUP);
  microServo.attach(servoPin);
  p = 0;
  // I2C LCD를 초기화 합니다..
  lcd.init();
  // I2C LCD의 백라이트를 켜줍니다.
  lcd.backlight();
}

//ph값
double avergearray(int* arr, int number) {
  int i;
  int max, min;
  double avg;
  long amount = 0;
  if (number <= 0) {
    Serial.println("Error number for the array to avraging!/n");
    return 0;
  }
  if (number < 5) { //less than 5, calculated directly statistics
    for (i = 0; i < number; i++) {
      amount += arr[i];
    }
    avg = amount / number;
    return avg;
  } else {
    if (arr[0] < arr[1]) {
      min = arr[0]; max = arr[1];
    }
    else {
      min = arr[1]; max = arr[0];
    }
    for (i = 2; i < number; i++) {
      if (arr[i] < min) {
        amount += min;      //arr<min
        min = arr[i];
      } else {
        if (arr[i] > max) {
          amount += max;  //arr>max
          max = arr[i];
        } else {
          amount += arr[i]; //min<=arr<=max
        }
      }//if
    }//for
    avg = (double)amount / (number - 2);
  }//if
  return avg;
}


//버튼버튼버튼--------------------------------------------------------------------------
bool swup = HIGH;
bool swdown = HIGH;
bool swleft = HIGH;
bool swright = HIGH;
bool swyes = HIGH;

bool swup_press = HIGH;
bool swdown_press = HIGH;
bool swleft_press = HIGH;
bool swright_press = HIGH;
bool swyes_press = HIGH;

bool swup_prev = LOW;
bool swdown_prev = LOW;
bool swleft_prev = LOW;
bool swright_prev = LOW;
bool swyes_prev = LOW;
unsigned long btn_last_time1;
unsigned long btn_last_time2;
unsigned long btn_last_time3;
unsigned long btn_last_time4;
unsigned long btn_last_time5;
//버튼----------------------------------------------------------------------------
float h;
float t;
float f;
float hic;
float hif;
int hit;
static float pHValue;

void loop() {
  watervalue = pos[0]; //물주가 변수
  hitvalue = pos[1]; //물주기 온도변수
  fanhit = pos[2]; //팬온도
  fanh = pos[3]; // 팬습도
  phValue = pos[4];
  
  nowtime = millis();
  if ((nowtime - lasttime) >= 1000) {
    h = dht.readHumidity(); //습도값을 h에 저장
    t = dht.readTemperature(); //온도값을 t에 저장
    f = dht.readTemperature(true);// 화씨 온도를 측정합니다.
    hif = dht.computeHeatIndex(f, h);
    hic = dht.computeHeatIndex(t, h, false);
    hit = hic;
    Serial.print("Humidity: "); //문자열 출력
    Serial.print(h); //습도값 출력
    Serial.print("% ");
    //DigitShield.setValue(h);
    lasttime = millis();

    //
    Serial.print("Temperature: ");
    Serial.print(hic); //온도값 출력
    Serial.println("C");
    //DigitShield.setValue(t);
    // delay(500);
    water = analogRead(A3);           //토양 습도 센서 아날로그핀 A3 번 사용 , A3 값 읽어와 water에 저장
    Serial.print("Soil humidity: ");  //토양 습도 체크
    Serial.println(water);

    //ph값 측정 및 출력
    static unsigned long samplingTime = millis();
    static unsigned long printTime = millis();
    static float voltage;
    if (millis() - samplingTime > samplingInterval)
    {
      pHArray[pHArrayIndex++] = analogRead(SensorPin);
      if (pHArrayIndex == ArrayLenth)pHArrayIndex = 0;
      voltage = avergearray(pHArray, ArrayLenth) * 5.0 / 1024;
      pHValue = 3.5 * voltage + Offset;
      samplingTime = millis();
    }
    if (millis() - printTime > printInterval)  //Every 800 milliseconds, print a numerical, convert the state of the LED indicator
    {
      Serial.print("    pH value: ");
      Serial.println(pHValue, 2);
      printTime = millis();
    }


    //양분공급 모터 동작 조건
    if (pHValue > 5) { //측정한 pHValue(ph값)이 5.0 이상일 경우 양분 부족
      for ( angle = 0; angle < 170; angle++)
      {
        microServo.write(angle);
        //delay(10);
      }
      for (angle = 170; angle > 0; angle--)
      {
        microServo.write(angle);
        // delay(10);
      }
      Serial.println("양분모터 ON");
    }

    //스페이스 히터 동작 조건
    if (hit > hitvalue)
    {
      digitalWrite(hitter, LOW);
      Serial.println("hitter off");
    }
    else
    {

      digitalWrite(hitter, HIGH);
      Serial.println("hitter on");
    }

    // 수분 공급 모터 동작 조건
    if ( water > watervalue)   // 토양 습도 체크 800보다s 낮으면 모터 동작 시켜 물공급 아니면 정지
    {
      digitalWrite(motor, HIGH);
      Serial.println("motor on");
    }
    else   {
      digitalWrite(motor, LOW);
      Serial.println("motor off");
    }

    //쿨링팬 동작 조건
    if (hit >= 30 || h >= 80 ) // 내부 온습도 체크 온도가 50도 이상이거나 습도가 80 이상이면 쿨링팬 동작
    {
      digitalWrite(fan, HIGH);
      Serial.println("fan on");
    }
    else   {
      digitalWrite(fan, LOW);
      Serial.println("fan off");
    }
    Serial.print("p값:");
    Serial.println(p);
    Serial.println(pos[0]);
    Serial.println(pos[1]);
    Serial.println(pos[2]);
    Serial.println(pos[3]);
    Serial.println("센서변수값");
    Serial.println(watervalue);
    Serial.println(hitvalue);
    Serial.println(fanhit);
    Serial.println(fanh);

  }

  //버튼예시
  int btnvalue = btn();
  control(btnvalue);

  //lcd display
  lcdDisplay(p,btnvalue);
  

  //Serial.print(btnvalue);
  //Serial.println("=버튼값 ");
  //버튼예시
}

void control(int btn) {
  int arrayvalue;
  if (btn == 1)
  {
    p = p - 1;
  }
  else if (btn == 2)
  {
    p = p + 1;
  }
  else if (btn == 3)
  {
    pos[p] -= 5;
  }
  else if (btn == 4)
  {
    pos[p] += 1;
  }

  if (p < 0)
  {
    p = 4;
  }
  else if (p > 4)
  {
    p = 0;
  }
  if (btn == 5)
  {
  }
  //lcdDisplay(p);
}

int btn() {
  change = true;
  swup = digitalRead(btnvalueup);     // 현재 switch 상태를 읽습니다.
  swdown = digitalRead(btnvaluedown);     // 현재 switch 상태를 읽습니다.
  swleft = digitalRead(btnposleft);     // 현재 switch 상태를 읽습니다.
  swright = digitalRead(btnposright);     // 현재 switch 상태를 읽습니다.
  swyes = digitalRead(btnyes);     // 현재 switch 상태를 읽습니다.

  if ((swup == LOW) && (swup_prev == HIGH))     // 센서종류+
  {
    swup_press = HIGH;                // 스위치가 눌렸음을 알림
    btn_last_time1 = millis();                     // 스위치가 눌린 시간을 기록
  }
  swup_prev = swup;     // 현재 switch 상태를 다음 loop의 이전 상태로 저장

  if ((swup_press == HIGH) && ((millis() - btn_last_time1) > 100)) // 50ms 이상 지났으면 실행
  {
    Serial.print("버튼1 \n");
    swup_press = LOW;
    return 1;
  }
  //-------------------------------------------------------------------------------
  if ((swdown == LOW) && (swdown_prev == HIGH))     // 센서종류-
  {
    swdown_press = HIGH;                // 스위치가 눌렸음을 알림
    btn_last_time2 = millis();                     // 스위치가 눌린 시간을 기록
  }
  swdown_prev = swdown;     // 현재 switch 상태를 다음 loop의 이전 상태로 저장

  if ((swdown_press == HIGH) && ((millis() - btn_last_time2) > 100)) // 50ms 이상 지났으면 실행
  {
    Serial.print("버튼2 \n");
    swdown_press = LOW;
    return 2;
  }
  //--------------------------------------------------------------------------------
  if ((swleft == LOW) && (swleft_prev == HIGH))     //  센서값+
  {
    swleft_press = HIGH;                // 스위치가 눌렸음을 알림
    btn_last_time3 = millis();                     // 스위치가 눌린 시간을 기록
  }
  swleft_prev = swleft;     // 현재 switch 상태를 다음 loop의 이전 상태로 저장

  if ((swleft_press == HIGH) && ((millis() - btn_last_time3) > 100)) // 50ms 이상 지났으면 실행
  {
    Serial.print("버튼3 \n");
    swleft_press = LOW;
    return 3;
  }
  //-------------------------------------------------------------------------------------------
  if ((swright == LOW) && (swright_prev == HIGH))     //센서값-
  {
    swright_press = HIGH;                // 스위치가 눌렸음을 알림
    btn_last_time4 = millis();                     // 스위치가 눌린 시간을 기록
  }
  swright_prev = swright;     // 현재 switch 상태를 다음 loop의 이전 상태로 저장

  if ((swright_press == HIGH) && ((millis() - btn_last_time4) > 100)) // 50ms 이상 지났으면 실행
  {
    Serial.print("버튼4 \n");
    swright_press = LOW;
    return 4;
  }
  //------------------------------------------------------------------------------------------
  if ((swyes == LOW) && (swyes_prev == HIGH))     //  입력 시
  {
    swyes_press = HIGH;                // 스위치가 눌렸음을 알림
    btn_last_time5 = millis();                     // 스위치가 눌린 시간을 기록
      change = false;
   lcdChangePage(change);
  }
  swyes_prev = swyes;     // 현재 switch 상태를 다음 loop의 이전 상태로 저장
  if ((swyes_press == HIGH) && ((millis() - btn_last_time5) > 100))
  {
    Serial.print("버튼5 \n");
    swyes_press = LOW;
    return 5;
  }
  return 0;
}

void lcdDisplay(int btnvalue, int btnYes) {
  static bool change= false;
  if(btnYes==5)
  {
    change=!change;
     lcd.clear();
  }
  if(change ==false)
  {
    lcd.setCursor(0, 0);
  lcd.print("Soil");

  lcd.setCursor(6, 0);
  lcd.print("pH");

  lcd.setCursor(12, 0);
  lcd.print("Hitter");

  lcd.setCursor(0, 2);
  lcd.print("Fan H");

  lcd.setCursor(12, 2);
  lcd.print("Fan T");
  switch (btnvalue)  {
    case 0:
    
      lcd.setCursor(0, 1);
      lcd.print(">");
      lcd.setCursor(1, 1);
      lcd.print(watervalue);
      
      lcd.setCursor(12, 1);
      lcd.print(" ");
      lcd.print(hitvalue);
      
      lcd.setCursor(6, 1);
      lcd.print(" ");
      lcd.print(phValue);
      
      lcd.setCursor(0, 3);
      lcd.print(" ");
      lcd.print(fanh);
      
      lcd.setCursor(12, 3);
      lcd.print(" ");
      lcd.print(fanhit);
      break;
    case 1:
    
      lcd.setCursor(12, 1);
      lcd.print(">");
      lcd.setCursor(13, 1);
      lcd.print(hitvalue);
      
      lcd.setCursor(0, 1);
      lcd.print(" ");
      lcd.print(watervalue);
      
      lcd.setCursor(0, 3);
      lcd.print(" ");
      lcd.print(fanh);
      
      lcd.setCursor(6, 1);
      lcd.print(" ");
      lcd.print(phValue);
      
      lcd.setCursor(12, 3);
      lcd.print(" ");
      lcd.print(fanhit);
      break;
    case 2:
    
      lcd.setCursor(12, 3);
      lcd.print(">");
      lcd.setCursor(13, 3);
      lcd.print(fanhit);

      lcd.setCursor(12, 1);
      lcd.print(" ");
      lcd.print(hitvalue);

      lcd.setCursor(0, 1);
      lcd.print(" ");
      lcd.print(watervalue);

      lcd.setCursor(0, 3);
      lcd.print(" ");
      lcd.print(fanh);

      lcd.setCursor(6, 1);
      lcd.print(" ");
      lcd.print(phValue);
      break;
    case 3:
    
      lcd.setCursor(0, 3);
      lcd.print(">");
      lcd.setCursor(1, 3);
      lcd.print(fanh);

      lcd.setCursor(12, 1);
      lcd.print(" ");
      lcd.print(hitvalue);

      lcd.setCursor(0, 1);
      lcd.print(" ");
      lcd.print(watervalue);

      lcd.setCursor(6, 1);
      lcd.print(" ");
      lcd.print(phValue);

      lcd.setCursor(12, 3);
      lcd.print(" ");
      lcd.print(fanhit);
      break;
    case 4:
    
      lcd.setCursor(6, 1);
      lcd.print(">");
      lcd.setCursor(7, 1);
      lcd.print(phValue);

      lcd.setCursor(12, 1);
      lcd.print(" ");
      lcd.print(hitvalue);

      lcd.setCursor(0, 1);
      lcd.print(" ");
      lcd.print(watervalue);

      lcd.setCursor(0, 3);
      lcd.print(" ");
      lcd.print(fanh);

      lcd.setCursor(12, 3);
      lcd.print(" ");
      lcd.print(fanhit);
    break;
    default:
      lcd.setCursor(0, 0);
      lcd.print("No Button Click!");
  }
  }
  else{
   
    lcd.setCursor(0,0);
    lcd.print("Temperature");
    lcd.setCursor(12,0);
    lcd.print("PH");
    
    
    lcd.setCursor(2,1);
    lcd.print(hic);
    lcd.setCursor(5,1);
    lcd.print("C");
    lcd.setCursor(13,1);
    lcd.print(pHValue);

    
    lcd.setCursor(0,2);
    lcd.print("Humidity");
    lcd.setCursor(10,2);
    lcd.print("Soil");

    lcd.setCursor(0,3);
    lcd.print(h);
    lcd.setCursor(10,3);
    lcd.print(water);
    
    
  }
  
  
  
}

int lcdChangePage(int yesBtn) {
  
  
}
