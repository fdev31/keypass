#ifndef _PASS_H
#define _PASS_H

Password passwords[] = {
    {FLAG_NONE, "first PASS 12 !"},
    {FLAG_NONE, "second pass 34"},
    {FLAG_NONE, "third pass foo() #@%!"},
    {FLAG_NONE, "fourth pass"},
    {FLAG_NONE, "this one is very long..."},
};

#define NB_PASSWORDS 5

#endif
