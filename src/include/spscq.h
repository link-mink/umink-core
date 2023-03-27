#ifndef SPSCQ_H
#define SPSCQ_H

#include <stdint.h>
#include <stdlib.h>
#include <math.h>

/** memory barrier */
#define mb() __asm__ __volatile__("" : : : "memory")

/**
 * SPSC Queue
 *
 * cache line is 64 bytes on x86 and armv7/v8;
 * the point of all this padding is to prevent
 * two cores reading from the same cacheline;
 * each struct member occupies one cacheline
 */
typedef struct {
    uint64_t shadow_head;
    unsigned char __cacheline_padding_1[56];
    volatile uint64_t head;
    unsigned char __cacheline_padding_2[56];
    volatile uint64_t tail;
    unsigned char __cacheline_padding_3[56];
    uint64_t shadow_tail;
    unsigned char __cacheline_padding_4[56];
    void **items;

} spsc_queue_t;

/**
 * SPSC Queue Descriptor
 */
typedef struct {
    int QSIZE;
    int QUEUE_ITEMS_MASK;
    int CACHE_LINE_LEN;
    int QUEUE_WATERMARK;
    int QUEUE_WATERMARK_MASK;
    spsc_queue_t q;

} spsc_qd_t;

/**
 * Create new queue
 * @param[in]   qsize   Queue size
 * @return      Pointer to queue descriptor
 */
static inline spsc_qd_t *
spscq_new(int qsize)
{
    // new q descriptor
    spsc_qd_t *qd = (spsc_qd_t *)calloc(1, sizeof(spsc_qd_t));
    if (!qd) {
        return 0;
    }
    // init
    if (qsize < 256) {
        qsize = 256;
    }
    qd->QSIZE = (int)pow((double)2, (int)ceil(log10(qsize + 1) / log10(2)));
    qd->QUEUE_ITEMS_MASK = qd->QSIZE - 1;
    qd->CACHE_LINE_LEN = 64;
    qd->QUEUE_WATERMARK = 256; // pow of 2
    qd->QUEUE_WATERMARK_MASK = qd->QUEUE_WATERMARK - 1;
    // q pointer
    spsc_queue_t *q = &qd->q;
    // setup
    q->tail = q->shadow_tail = qd->QSIZE - 1;
    q->head = q->shadow_head = 0;
    // alloc q
    q->items = (void **)malloc(sizeof(void *) * qd->QSIZE);
    // return qd
    return qd;
}
/**
 * Destroy spsc queue
 * @param[in]   qd      Pointer to queue descriptor
 */
static inline void
spscq_free(spsc_qd_t *qd)
{
    free(qd->q.items);
    free(qd);
}

/**
 * Pop item from queue
 * @param[in]       qd      Pointer to queue descriptor
 * @param[in,out]   item    Popped item
 * @return          0 for success
 */
static inline int
spscq_pop(spsc_qd_t *qd, void **item)
{
    uint32_t next_tail;
    next_tail = (qd->q.shadow_tail + 1) & qd->QUEUE_ITEMS_MASK;

    if (next_tail != qd->q.head) {
        *item = qd->q.items[next_tail];
        qd->q.shadow_tail = next_tail;

        if ((qd->q.shadow_tail & qd->QUEUE_WATERMARK_MASK) == 0) {
            mb();
            qd->q.tail = qd->q.shadow_tail;
        }
        return 0;
    }
    return 1;
}

/**
 * Push item to queue
 * @param[in]   qd              Pointer to queue descriptor
 * @param[in]   flush_item      If 1, i
 * @param[in]   item            Item to push to queu
 * @return      0 for success
 */
static inline int
spscq_push(spsc_qd_t *qd, uint8_t flush_item, void *item)
{
    uint32_t next_head;
    next_head = (qd->q.shadow_head + 1) & qd->QUEUE_ITEMS_MASK;

    if (qd->q.tail != next_head) {
        qd->q.items[qd->q.shadow_head] = item;
        qd->q.shadow_head = next_head;

        if (flush_item || (qd->q.shadow_head & qd->QUEUE_WATERMARK_MASK) == 0) {
            mb();
            qd->q.head = qd->q.shadow_head;
        }
        return 0;
    }
    return 1;
}

#endif /* end of include guard: SPSCQ_H */
