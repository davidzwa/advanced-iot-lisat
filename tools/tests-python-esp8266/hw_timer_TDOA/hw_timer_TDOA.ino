#define TIMEOUT 47000          //80 ticks per us from TIM_DIV1, at a distance of 20cm, max delay = (0.15/343)*10^6 * 80 ~= 47000 ticks // Max 8388607
#define NUM_MIC 2

#define MIC0_GPIO 12  //D6
#define MIC1_GPIO 4   //D2
#define MODE_GPIO 5   //D1

int mic_done = 0;

bool toggle = true;
bool timeout_flag = false;

uint32_t t_mic[NUM_MIC];
int32_t TDOA[NUM_MIC-1];


void inline en_timer1() //here to ensure all timers are enabled in the same way
{
  timer1_enable(TIM_DIV1, TIM_EDGE, TIM_LOOP);
}

void ICACHE_RAM_ATTR timerout()
{
  timeout_flag = true;
}

void inline mic_handler(int mic){

  if(timer1_enabled() && !(mic_done & (1 << mic)))  //check if timer is already running and that this mic hasn't fired yet
  {
    t_mic[mic] = timer1_read();  //store timer value
    mic_done = mic_done | (1 << mic); //set this mic as done   
  }
  else
  { 
    timer1_write(TIMEOUT); 
    en_timer1();
    t_mic[mic] = TIMEOUT;
    mic_done = mic_done | (1 << mic);        
  }
  Serial.println(mic);
}

void ICACHE_RAM_ATTR mic0_handler(){
  mic_handler(0);

  toggle = !toggle;
  digitalWrite(LED_BUILTIN, toggle);
  detachInterrupt(digitalPinToInterrupt(MIC0_GPIO));
}

void ICACHE_RAM_ATTR mic1_handler(){
  mic_handler(1);
  detachInterrupt(digitalPinToInterrupt(MIC1_GPIO));
}

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);

  timer1_isr_init();
  timer1_attachInterrupt(timerout);
  
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(MODE_GPIO, OUTPUT);
  pinMode(MIC0_GPIO, INPUT);
  pinMode(MIC1_GPIO, INPUT);
  
  digitalWrite(LED_BUILTIN, toggle);
  digitalWrite(MODE_GPIO, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(MIC0_GPIO), mic0_handler, RISING);
  attachInterrupt(digitalPinToInterrupt(MIC1_GPIO), mic1_handler, RISING);
  
}



void loop() {
  // put your main code here, to run repeatedly:


  if( mic_done == B11) //all mics have triggered
  {
    timer1_disable();

    /*Serial.println("event detected");
    Serial.print("T mic0: ");
    Serial.println(t_mic[0]);*/

    for(int i = 1; i < NUM_MIC; i++)
    {
      TDOA[i] = t_mic[0] - t_mic[i];
      Serial.print("T mic");
      Serial.print(i);
      Serial.print(": ");
      Serial.print(t_mic[i]);
      Serial.print("TDOA: ");
      Serial.println(TDOA[i]);

      t_mic[i] = 0;
    }

    t_mic[0] = 0;
    mic_done = 0;

    // reset mics
    digitalWrite(MODE_GPIO, LOW);
    delay(100);
    digitalWrite(MODE_GPIO, HIGH);    

    attachInterrupt(digitalPinToInterrupt(MIC0_GPIO), mic0_handler, RISING);
    attachInterrupt(digitalPinToInterrupt(MIC1_GPIO), mic1_handler, RISING);
  }

  if(timeout_flag)
  {
    Serial.println("Timer timeout");

    digitalWrite(MODE_GPIO, LOW);

    for(int i = 0; i < NUM_MIC; i++)
    {
      t_mic[i] = 0;
    }

    mic_done = 0;
    
    timeout_flag = false;
    timer1_disable();
    digitalWrite(MODE_GPIO, HIGH);  
  }

  attachInterrupt(digitalPinToInterrupt(MIC0_GPIO), mic0_handler, RISING);
  attachInterrupt(digitalPinToInterrupt(MIC1_GPIO), mic1_handler, RISING);



}
