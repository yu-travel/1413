
#ifndef __SOFTTIMER_H_
#define __SOFTTIMER_H_

#include "types_def.h"
#include "MultiTimer.h"


typedef MultiTimer          _time_t;

int softtimer_start(_time_t *timer, uint64_t period_ms, MultiTimerCallback_t callback, void *userdata);
int softtimer_stop(_time_t *timer);
int softtimer_init(void);
void softtimer_loop(void);


#endif  /* __SOFTTIMER_H_ */

