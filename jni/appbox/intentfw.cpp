#include <binder/Parcel.h>
#include <string.h>
#include <utils/String16.h>
#include <utils/String8.h>
#include "intentfw.h"
#include "ptraceaux.h"
#include "binder.h"
#include "log.h"

static void freeBuffer(android::Parcel* parcel, const uint8_t* data, size_t dataSize,
                                const size_t* objects, size_t objectsSize,
                                void* cookie)
{

}

void handle_intent(pid_t pid, struct binder_transaction_data *txn)
{
    android::Parcel parcel;
    uint8_t buffer[txn->data_size];
    ptrace_read_data(pid, buffer, (tracee_ptr_t) txn->data.ptr.buffer, txn->data_size);
    size_t noffsets = (size_t) (txn->offsets_size / sizeof(size_t));
    size_t offsets[noffsets];
    ptrace_read_data(pid, offsets, (tracee_ptr_t) txn->data.ptr.offsets, txn->offsets_size);

    parcel.ipcSetDataReference(
	buffer,
        (size_t) txn->data_size,
        offsets,
        noffsets, &freeBuffer, NULL);
    int32_t policy = parcel.readInt32();
    android::String16 interfaceToken = parcel.readString16();
    android::String8 dup = android::String8(interfaceToken);
    LOGD("intercept intent for %s\n", dup.string());
    if (dup == ICONTENT_PROVIDER) {
    	LOGD("\t content provider\n");
    }
}


#if 0
//only handle transaction
struct binder_transaction_data data;
ptrace_read_data(pid, &data, cur, sizeof(struct binder_transaction_data));
cur += sizeof(struct binder_transaction_data);
/* if(data.code == GET_SERVICE_TRANSACTION){ */
int i;

tracee_ptr_t ptr = (tracee_ptr_t)data.data.ptr.buffer + 4;

// the request to service manager
long len;

// include/binder/parcel.cpp
ptrace_read_data(pid, &len, ptr, sizeof(long));
char16_t service[len + 1];
ptrace_read_data(pid, service, ptr + sizeof(long), sizeof(char16_t) * (len + 1));
ptr +=(sizeof(long) + 2 + (len + 1) * sizeof(char16_t));
/* printf("service name: %ld ---", len); */
/* for(i = 0; i < len; i++) { */
/* 	printf("%c",(char)service[i]); */
/* } */
/* printf("\n"); */

// TODO: identify service base on service name and handler different service separately

if(CONTENT_SANDBOX_ENABLED && strcmp12(service, ICONTENT_PROVIDER) == 0 || (strcmp12(service, IACTIVITY_MANAGER) == 0)){ //&& data.code == GET_CONTENT_PROVIDER_TRANSACTION)){
	char16_t all_data[data.data_size / 2];
	ptrace_read_data(pid, (void *)all_data, (tracee_ptr_t)data.data.ptr.buffer, data.data_size);
	int com = 0;
	for(i = 0; i < data.data_size / 2; i++){
		if(strpreg12(&all_data[i], SANDBOX_CONTENT_PROVIDER) == 0) {
			com = i;
			strchpre12(&all_data[i], SANDBOX_CONTENT_PROVIDER_FAKE_PREFIX);
		}
		printf("%c", all_data[i]);
	}
	printf("\n");
	if(com > 0){
		/* for(i = 0; i < data.data_size / 2; i++){ */
		/* 	printf("%c|", all_data[i]); */
		/* } */
		/* printf("\n"); */
		ptrace_write_data(pid, all_data, (tracee_ptr_t)  data.data.ptr.buffer, data.data_size);
	}
}
	/* printf("service name: "); */
	/* ptrace_read_data(pid, &len, (void *)ptr, sizeof(int)); */
	/* printf("%d ---- ", len); */
	/* char16_t service_name[len + 1]; */

/* } */
#endif
