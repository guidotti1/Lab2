#ifndef CONTAINER_H
#define CONTAINER_H

#include "xmlParser.h"
#include "item.h"
#include "trigger.h"
#include "extraFunctions.h"
#include <vector>
#include <iterator>
using namespace std;


class container
{
public:
    container();
    container(string s);
    container(XMLNode node);
    void setTriggers(XMLNode node);
    void setItems(XMLNode node);
    string getName();
    void readItems();
    container& operator = (const container& c);
    item checkItems(string input);
    void getItems(vector<item> & returnItems);
    void removeItem(string itemName);
    void addItem(item newItem);
    void getContainerTriggerComands(vector<string> & commands);
    trigger checkTriggersByCommand(string input);
    string getStatus();
    void updateTrigger(trigger newTrigger);
    trigger getTriggerWithoutCommand();
    void removeTriggerWithoutCommand();
    void addTrigger(trigger newTrigger);
    void updateStatus(string newStatus);

private :
    string name;
    string status;
    string accept;
    vector<item> items;
    vector<trigger> triggers;

};

#endif
