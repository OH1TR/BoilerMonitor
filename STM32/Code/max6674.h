#ifndef MAX6674_H_
#define MAX6674_H_

void Max6674Init();
void Max6674SetCalibration(int divider,int offset);
int Max6674Read();

#endif
