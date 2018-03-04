class Session{
  private: 
    String rfidNumber;//card number
    unsigned long initTime;
    unsigned long finishTime;
    unsigned long powerComsuption;
    unsigned long actualCurrent;
  public:
    Session(String rfidNumber, unsigned long initTime);
    void addPower();//add actualCurrent to the powerComsuption
    void actualCurrent(unsigned long currentMeasure);//store actualCurrent
    void endSesion(finishTime);//end session finishTime
    void postActualCurrent();
    void postFinalSession();
    bool compareCard(String toCompareCardId);
};

Session::Session(String rfidNumber, unsigned long actualTime)
{
  this->rfidNumber = rfidNumber;
  this->initTime = actualTime;
  this->finishTime = -1;
  this->powerComsuption = 0;
  this->actualCurrent = 0;
}

void Session::addPower()
{
  if(finishTime != -1){
    return;
  }
  //assuming that we have 230V
  this->powerComsuption = this->powerComsuption + (this->actualCurrent * 230);
}

void Session::actualCurrent(unsigned long currentMeasure)
{
  if(finishTime != -1){
    return;
  }
  this->actualCurrent = currentMeasure;
}

void Session::endSesion(unsigned long finishTime)
{
  if(finishTime != -1){
    return;
  }
  this->endSesion = finishTime;
}

bool Session::compareCard(String toCompareCardId)
{
  return this->rfidNumber.equals(toCompareCardId);
}

//TODO
void Session::postActualCurrent(){
  return;
}

//TODO
void Session::postActualCurrent(){
  return;
}