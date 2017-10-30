/* Copyright 2008 The Android Open Source Project
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <linux/types.h>
#include <stdbool.h>
#include <string.h>

#include <private/android_filesystem_config.h>

#include "binder.h"

#define HELLO_SVR_CMD_SAYHELLO     0
#define HELLO_SVR_CMD_SAYHELLO_TO  1
#define HELLO_SVR_CMD_SAYGOODBYE     2
#define HELLO_SVR_CMD_SAYGOODBYE_TO  3

/*
uint32_t svcmgr_lookup(struct binder_state *bs, uint32_t target, const char *name)
{
    uint32_t handle;
    unsigned iodata[512/4];
    struct binder_io msg, reply;

    bio_init(&msg, iodata, sizeof(iodata), 4);
    bio_put_uint32(&msg, 0);  // strict mode header
    bio_put_string16_x(&msg, SVC_MGR_NAME);
    bio_put_string16_x(&msg, name);

    if (binder_call(bs, &msg, &reply, target, SVC_MGR_CHECK_SERVICE))
        return 0;

    handle = bio_get_ref(&reply);

    if (handle)
        binder_acquire(bs, handle);

    binder_done(bs, &msg, &reply);

    return handle;
}
*/

int svcmgr_publish(struct binder_state *bs, uint32_t target, const char *name, void *ptr)
{
    int status;
    unsigned iodata[512/4];
    struct binder_io msg, reply;

    bio_init(&msg, iodata, sizeof(iodata), 4);
    bio_put_uint32(&msg, 0);  // strict mode header
    bio_put_string16_x(&msg, SVC_MGR_NAME);
    bio_put_string16_x(&msg, name);
    bio_put_obj(&msg, ptr);

    if (binder_call(bs, &msg, &reply, target, SVC_MGR_ADD_SERVICE))
        return -1;

    status = bio_get_uint32(&reply);

    binder_done(bs, &msg, &reply);

    return status;
}

void sayhello(void)
{
    static int cnt = 0;
    fprintf(stderr, "%s, sayhello cnt = %d\n", __FILE__, cnt++);
}

void sayhello_to(char *name)
{
    static int cnt = 0;
    fprintf(stderr, "%s, sayhello_to name = %s, cnt = %d\n", __FILE__, name, cnt++);
}

void saygoodbye(void)
{
    static int cnt = 0;
    fprintf(stderr, "%s, saygoodbye cnt = %d\n", __FILE__, cnt++);
}

void saygoodbye_to(char *name)
{
    static int cnt = 0;
    fprintf(stderr, "%s, saygoodbye_to name = %s, cnt = %d\n", __FILE__, name, cnt++);
}

int svcmgr_handler(struct binder_state *bs,
                   struct binder_transaction_data *txn,
                   struct binder_io *msg,
                   struct binder_io *reply)
{
    uint16_t *s;
    size_t len;
    uint32_t handle;
    uint32_t strict_policy;
    char name[32];
    int i;

    //ALOGI("target=%x code=%d pid=%d uid=%d\n",
    //  txn->target.handle, txn->code, txn->sender_pid, txn->sender_euid);

//    if (txn->target.handle != svcmgr_handle)
//        return -1;
    fprintf(stderr, "%s, txn->target.handle = %d\n", __FILE__, txn->target.handle);

    if (txn->code == PING_TRANSACTION)
    {
        fprintf(stderr, "txn->code == PING_TRANSACTION\n");
        return 0;
    }

    // Equivalent to Parcel::enforceInterface(), reading the RPC
    // header with the strict mode policy mask and the interface name.
    // Note that we ignore the strict_policy and don't propagate it
    // further (since we do no outbound RPCs anyway).
    strict_policy = bio_get_uint32(msg);

//    s = bio_get_string16(msg, &len);
//    fprintf(stderr, "%s, svcmgr_handler get string = %s\n", s);
//    if (s == NULL) {
//        return -1;
//    }

//    if ((len != (sizeof(svcmgr_id) / 2)) ||
//        memcmp(svcmgr_id, s, sizeof(svcmgr_id))) {
//        fprintf(stderr,"invalid id %s\n", str8(s, len));
//        return -1;
//    }

//    if (sehandle && selinux_status_updated() > 0) {
//        struct selabel_handle *tmp_sehandle = selinux_android_service_context_handle();
//        if (tmp_sehandle) {
//            selabel_close(sehandle);
//            sehandle = tmp_sehandle;
//        }
//    }

    switch(txn->code) {
    case HELLO_SVR_CMD_SAYHELLO:
        sayhello();
        break;
    case HELLO_SVR_CMD_SAYHELLO_TO:
        s = bio_get_string16(msg, &len);
        if (s == NULL) {
            return -1;
        }

        for(i = 0; i < len; i++)
            name[i] = s[i];
        name[i] = '\0';

        sayhello_to(name);

        bio_put_uint32(reply, 0);
        break;
    case HELLO_SVR_CMD_SAYGOODBYE:
        saygoodbye();
        break;
    case HELLO_SVR_CMD_SAYGOODBYE_TO:
        s = bio_get_string16(msg, &len);
        if (s == NULL) {
            return -1;
        }

        for(i = 0; i < len; i++)
            name[i] = s[i];
        name[i] = '\0';

        saygoodbye_to(name);

        bio_put_uint32(reply, 0);
        break;

    default:
        fprintf(stderr, "unknow txn->code\n");
        return -1;
    }

    return 0;
}

//unsigned token;

int main(int argc, char **argv)
{
    int fd;
    struct binder_state *bs;
    uint32_t svcmgr = BINDER_SERVICE_MANAGER;
    uint32_t handle;
    int ret;

    bs = binder_open(128*1024);
    if (!bs) {
        fprintf(stderr, "failed to open binder driver\n");
        return -1;
    }

    ret = svcmgr_publish(bs, svcmgr, "hello", (void *)100);
    if(ret)
    {
        fprintf(stderr, "publish hello err\n");
        return -1;
    }

    ret = svcmgr_publish(bs, svcmgr, "goodbye", (void *)101);
    if(ret)
    {
        fprintf(stderr, "publish goodbye err\n");
        return -1;
    }

    binder_loop(bs, svcmgr_handler);

    /*
    argc--;
    argv++;
    while (argc > 0) {
        if (!strcmp(argv[0],"alt")) {
            handle = svcmgr_lookup(bs, svcmgr, "alt_svc_mgr");
            if (!handle) {
                fprintf(stderr,"cannot find alt_svc_mgr\n");
                return -1;
            }
            svcmgr = handle;
            fprintf(stderr,"svcmgr is via %x\n", handle);
        } else if (!strcmp(argv[0],"lookup")) {
            if (argc < 2) {
                fprintf(stderr,"argument required\n");
                return -1;
            }
            handle = svcmgr_lookup(bs, svcmgr, argv[1]);
            fprintf(stderr,"lookup(%s) = %x\n", argv[1], handle);
            argc--;
            argv++;
        } else if (!strcmp(argv[0],"publish")) {
            if (argc < 2) {
                fprintf(stderr,"argument required\n");
                return -1;
            }
            svcmgr_publish(bs, svcmgr, argv[1], &token);
            argc--;
            argv++;
        } else {
            fprintf(stderr,"unknown command %s\n", argv[0]);
            return -1;
        }
        argc--;
        argv++;
    }
    */

    return 0;
}
