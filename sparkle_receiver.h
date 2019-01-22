struct button_message {
    uint8_t button;
    uint8_t event;
    uint16_t pendant_id;
};

button_message event;

struct sparkle_event {
    uint16_t pendant_id;
    uint32_t last_rx;
};

#define MAX_SPARKLES 20
sparkle_event sparkle_buffer[MAX_SPARKLES] = {0};

int find_sparkle(uint16_t from) {
  for (int i=0; i<MAX_SPARKLES; i++) {
    if (from == sparkle_buffer[i].pendant_id) {
        return i;
    }
    return -1;
  }
}

void receive_sparkle(uint16_t from) {
  int i = find_sparkle(from);
  if (-1 == i) {
    for (i=0; i<MAX_SPARKLES; i++) {
      if (0 == sparkle_buffer[i].pendant_id) {
         break;
      }
    }
  }
  if (i >= MAX_SPARKLES) {
      i = 0;
  }
  sparkle_buffer[i].pendant_id = from;
  sparkle_buffer[i].last_rx = millis();
}

void clear_sparkle(uint16_t from) {
  int i = find_sparkle(from);
  if (i > -1) {
    sparkle_buffer[i].pendant_id = 0;
    sparkle_buffer[i].last_rx = 0;
  }
}

void prune_sparkles(uint32_t threshold) {
  for (int i=0; i<MAX_SPARKLES; i++) {
    if (sparkle_buffer[i].last_rx < threshold) {
       sparkle_buffer[i].pendant_id = 0;
       sparkle_buffer[i].last_rx = 0;      
    }
  }  
}

int number_of_sparkles() {
  int result = 0;
  for (int i=0; i<MAX_SPARKLES; i++) {
    if (sparkle_buffer[i].pendant_id > 0) {
      result++;    
    }
  }
  return result;
}

