#define SPEAKER_OUT 25
#define CHANNEL_0 0

void sound_init(){
  if (SPEAKER_OUT > 0){
    ledcSetup(CHANNEL_0, 1000, 8);
    ledcAttachPin(SPEAKER_OUT, CHANNEL_0);
  }
}

void sound_ok(void){
  if (SPEAKER_OUT > 0) {
    ledcWriteTone(CHANNEL_0, 800);  delay(200);
    ledcWriteTone(CHANNEL_0, 500);  delay(200);
  }
}

void sound_ko(void){
  if (SPEAKER_OUT > 0) {
    ledcWriteTone(CHANNEL_0, 500);  delay(200);
    ledcWriteTone(CHANNEL_0, 800);  delay(200);
  }
}
