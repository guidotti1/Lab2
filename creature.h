#ifndef CREATURE_H
#define CREATURE_H

#include "attack.h"
#include "trigger.h"
#include "xmlParser.h"
using namespace std;


class creature
{
public :
    creature();
    creature(XMLNode node);
    void getAttack(XMLNode node);
    trigger getTrigger();

private :
    string name;
    string vulnerability;
    attack creatureAttack;
    trigger creatureTrigger;

};
#endif
