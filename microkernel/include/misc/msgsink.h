#ifndef _MISC_MSGSINK_H
#define _MISC_MSGSINK_H


struct msgsink
{
  struct msgsink *next;

  void *opaque;

  void (*putchar) (void *, char);
  void (*puts) (void *, const char *);
};

void msgsink_register (struct msgsink *);

#endif /* _MISC_MSGSINK_H */
