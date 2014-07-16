#ifndef INTENT_FIREWALL_H_
#define INTENT_FIREWALL_H_

#include <linux/binder.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

void handle_intent(pid_t pid, struct binder_transaction_data *txn);

#ifdef __cplusplus
} /* extern C */
#endif

#endif /* INTENT_FIREWALL_H_ */
