
static const int CPU_ADDR_PINS[16] = 
{
  22, 24, 26, 28, 30, 32, 34, 36,
  38, 40, 42, 44, 46, 48, 50, 52
};

static const int CPU_DATA_PINS[8] = 
{
  23, 25, 27, 29, 31, 33, 35, 37
};

static const int CPU_RES = A7;
static const int CPU_RDY = A8;
static const int CPU_PHI2 = A9;
static const int CPU_IRQB = A10;
static const int CPU_MLB = A11;
static const int CPU_NMIB = A12;
static const int CPU_SYNC = A13;
static const int CPU_BE = A14;
static const int CPU_RWB = A15;

uint8_t cpu_data_val = 0;

uint8_t rom[] = 
{
  0x18,             // clc
  0x92, 0x00,       // sta (0)
  0x69, 0x01,       // adc #1
  0x4c, 0x00, 0x80  // jmp $8000
};

uint8_t isr_vector[] =
{
  0x00, 0x80, // res
  0x00, 0x80, // brk/irq
};


uint16_t cpu_read_addr()
{
  uint16_t addr = 0;
  for(int i = 0; i < 16; ++i)
    addr |= (digitalRead(CPU_ADDR_PINS[i]) ? 1 : 0) << i;
  return addr;
}

uint8_t cpu_read_data()
{
  uint16_t data = 0;
  for(int i = 0; i < 8; ++i)
  {
    pinMode(CPU_DATA_PINS[i], INPUT);
    data |= (digitalRead(CPU_DATA_PINS[i]) ? 1 : 0) << i;
  }
  cpu_data_val = data;
  return data;
}

void cpu_write_data(uint8_t data)
{
  for(int i = 0; i < 8; ++i)
  {
    pinMode(CPU_DATA_PINS[i], OUTPUT);
    digitalWrite(CPU_DATA_PINS[i], !!(data & (1 << i)));
  }
  cpu_data_val = data;
}

void cpu_log_pins()
{
  uint16_t addr = cpu_read_addr();
  uint8_t data = cpu_data_val;

  char buf[64];
  sprintf(buf, "Addr: $%04x, Data: $%02x, R/W: %s\n", addr, data, digitalRead(CPU_RWB) ? "r" : "w");
  Serial.write(buf);
}

void cpu_tick()
{
  digitalWrite(CPU_PHI2, 1);

  uint16_t addr = cpu_read_addr();

  if(digitalRead(CPU_RWB))
  {
    if(addr >= 0xfffc && addr <= 0xffff)
    {
      cpu_write_data(isr_vector[addr - 0xfffc]);
    }
    else if(addr >= 0x8000 && addr <= (0x8000 + sizeof(rom)))
    {
      cpu_write_data(rom[addr - 0x8000]);
    }
    else
    {
      cpu_write_data(0x00); // nop
    }
  }
  else
  {
    cpu_read_data();

    if(addr >= 0x0000 && addr <= 0x7fff)
    {
      char buf[64];
      sprintf(buf, " - Write @ $%04x: $%02x\n", addr, cpu_data_val);
      Serial.write(buf);
    }
  }
  
  digitalWrite(CPU_PHI2, 0);
}

void cpu_reset() {
  Serial.write("Reset 6502...\n");
  digitalWrite(CPU_RES, 0);
  for(int i = 0; i < 8; ++i)
    cpu_tick();
  digitalWrite(CPU_RES, 1);
}

void setup() {
  Serial.begin(115200);
  
  // put your setup code here, to run once:
  pinMode(CPU_RES, OUTPUT);
  digitalWrite(CPU_RES, 1);

  pinMode(CPU_RDY, OUTPUT);
  digitalWrite(CPU_RDY, 1);
  
  pinMode(CPU_PHI2, OUTPUT);
  digitalWrite(CPU_PHI2, 0);

  pinMode(CPU_IRQB, OUTPUT);
  digitalWrite(CPU_IRQB, 1);

  pinMode(CPU_MLB, OUTPUT);
  digitalWrite(CPU_MLB, 0);
  
  pinMode(CPU_NMIB, OUTPUT);
  digitalWrite(CPU_NMIB, 1);
  
  pinMode(CPU_SYNC, INPUT);

  pinMode(CPU_BE, OUTPUT);
  digitalWrite(CPU_BE, 1);

  pinMode(CPU_RWB, INPUT);
  
  for(int i = 0; i < 16; ++i)
    pinMode(CPU_ADDR_PINS[i], INPUT);
  for(int i = 0; i < 8; ++i)
    pinMode(CPU_DATA_PINS[i], INPUT);

  cpu_reset();

  Serial.write("Begin run...\n");
}

void loop() {
  cpu_tick();
  //cpu_log_pins();
}
