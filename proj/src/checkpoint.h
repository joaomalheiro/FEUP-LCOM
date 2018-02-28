#ifndef _CHECKPOINT_H
#define _CHECKPOINT_H

/** @defgroup checkpoint checkpoint
 * @{
 *
 * Module that implements Checkpoints, for use with Guards
 */

//Guard checkpoints for point cycling
typedef struct {
  long x;
  long y;
  short speed;
} Checkpoint;

/**
 * @brief Creates a checkpoint based on the passed in arguments
 * @param  x     X of the checkpoint to be created
 * @param  y     Y of the checkpoint to be created
 * @param  speed Speed from this checkpoint to the next one
 * @return       Returns a pointer to a valid checkpoint or NULL in case of failure
 */
Checkpoint * create_checkpoint(long x, long y, short speed);

/**
 * @brief Destroys the passed in checkpoint, deallocating the memory and setting the passed pointer to NULL
 * @param checkpoint_ptr Checkpoint to destroy
 */
void destroy_checkpoint(Checkpoint ** checkpoint_ptr);

#endif /* __CHECKPOINT_H */
