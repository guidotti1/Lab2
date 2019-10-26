#include <string>
#include <iostream>
#include <sstream>
#include <vector>

#define XML_USE_STL
#include "xmlParser.h"
#include "room.h"
#include "creature.h"
#include "attack.h"
#include "container.h"
#include "border.h"
#include "item.h"
#include "trigger.h"
#include "turnon.h"
#include "condition.h"
#include "player.h"
#include <map>
#include <iostream>
#include <string>
#include <iterator>

using namespace std;


void activateCreatureTriggers(room & current, player user);
void activateContainerTriggers(room & current, player user);
bool triggersInRoom(room current, string userinput);
bool triggersInContainers(room current, string userinput, container & triggerPresentContainer);
void activateRoomCommandTriggers(room current, string userinput, player user, bool & triggersPresentInContainers, bool & triggersPresentInRoom);
void activateContainerCommandTriggers(container triggerPresentContainer, string userinput, bool & triggersPresentInContainers, bool & triggersPresentInRoom);


int main (int argc, char ** argv) {

    string gameFile = "fullGame.xml";

    // this opens and parses the correct XML file:
    XMLNode xMainNode=XMLNode::openFileHelper(gameFile.c_str(),"map");
    int i = 0;
    // get the first room node
    XMLNode roomNode=xMainNode.getChildNode(i++);
    map<string, room> roomMap;

    do {
    //make map of rooms indexed by room name
    room newRoom(roomNode);
    string roomName = newRoom.getName();
    roomMap[roomName] = newRoom;
    roomNode=xMainNode.getChildNode(i++);
    } while (!roomNode.isEmpty());

    room entrance = roomMap["Entrance"];
    player user(entrance);
    bool foundExit = false;
    while (foundExit == false)
        {
        bool changedRooms = false;
        room current = user.currentLocation();
        current.readDescription();
        activateCreatureTriggers(current, user);
        activateContainerTriggers(current, user);

        string userinput;
        getline(cin, userinput);
        vector<string> inputVect;
        separateWords(userinput, inputVect);


        //all this is to activate triggers that occur because of user commands.
        //for the room, we check if the user has a certain item or not
        //for the containers, we check the status of said container (e.g. locks on a door or whatnot)
        bool triggersPresentInRoom = triggersInRoom(current, userinput);
        container triggerPresentContainer;
        bool triggersPresentInContainers = triggersInContainers(current, userinput, triggerPresentContainer);

        if ((triggersPresentInRoom) && !(triggersPresentInContainers))
            {
            activateRoomCommandTriggers(current, userinput, user, triggersPresentInContainers, triggersPresentInRoom);
            }

        else if ((triggersPresentInContainers) && !(triggersPresentInRoom))
            {
            activateContainerCommandTriggers(triggerPresentContainer, userinput, triggersPresentInContainers, triggersPresentInRoom);
            }
        //end of room and container triggers (activated by user commands)

        if (!(triggersPresentInContainers) && !(triggersPresentInRoom))
            {
                if (inputVect.size() == 1)
                {
                    userinput = inputVect[0];
                    if ((userinput == "n") || (userinput == "s") || (userinput == "e") || (userinput == "w")) //change user's current location to n, s ,e, w if there are rooms in those directions
                        {
                        user.moveToBorder(roomMap, changedRooms, userinput);
                        }
                    else if (userinput == "i")
                        {
                        user.readInventory();
                        }
                    else if (userinput == "quit")
                        {
                        foundExit = true;
                        }
                }
                else if (inputVect.size() == 2)
                    {
                        if (inputVect[0] == "take") //user takes an item if it's there
                            {
                            string passItemName = inputVect[1];
                            user.userTakeItem(current, passItemName);
                            }

                        else if (inputVect[0] == "read") //user reads an item if they have it in inventory
                            {
                            string passItemName = inputVect[1];
                            user.userReadItem(passItemName);
                            }

                        else if (inputVect[0] == "drop") //user drops an item into the room
                            {
                            string dropItemName = inputVect[1];
                            user.userDropItem(current, dropItemName);
                            }

                        else if (inputVect[0] == "open")
                            {
                            string secondWord = inputVect[1];
                            if (secondWord == "exit")
                                {
                                user.isAtExit (foundExit); //terminates the game if the user has made it to the exit
                                }
                            else
                                {
                                user.userOpenContainer(secondWord); //user opens a container
                                }
                            }
                        }
                    else if (inputVect.size() == 3)
                        {
                            if ((inputVect[0] == "turn") && (inputVect[1] == "on")) //user turns on an item
                                {
                                string turnonItem = inputVect[2];
                                user.userTurnonItem(turnonItem);
                                }
                        }
                    else if (inputVect.size() == 4)
                        {
                            if ((inputVect[0] == "put") && (inputVect[2] == "in"))
                                {
                                string itemName = inputVect[1];
                                string containerName = inputVect[3];
                                user.userPutItemInContainer(current, itemName, containerName);
                                }
                            if ((inputVect[0] == "attack") && (inputVect[2] == "with"))
                                {
                                string creatureName = inputVect[1];
                                string itemName = inputVect[3];
                                user.userAttackCreature(creatureName, itemName, current);
                                }
                        }
                }
        if (changedRooms == false)
            {
            user.updateLocation(current);
            }
        }
    return 0;
}


void activateCreatureTriggers(room & current, player user)
{
    vector<creature> roomCreatures;
    current.getCreatures(roomCreatures);
    for (int i = 0; i < roomCreatures.size(); i++)
        {
            trigger creatureTrigger = roomCreatures[i].getTrigger();
            if (creatureTrigger.getActivated() == false)
                {
                condition creatureTriggerCondition = creatureTrigger.getCondition();
                string object, status, owner;
                object = creatureTriggerCondition.getObject();
                status = creatureTriggerCondition.getStatus();
                owner = creatureTriggerCondition.getOwner();
                if (owner == "inventory")
                    {
                        item returnItem = user.checkItems(object);
                        if (returnItem.getName() == object)
                            {
                            if (returnItem.getStatus() == status)
                                {
                                creatureTrigger.executePrint();
                                creatureTrigger.updateActivated();
                                roomCreatures[i].updateTrigger(creatureTrigger);
                                }
                            }
                    }
                current.removeCreature(roomCreatures[i].getName());
                current.addCreature(roomCreatures[i]);
                }
        }
}

void activateContainerTriggers(room & current, player user)
{
    vector<container> roomContainers;
    current.getContainers(roomContainers);
    for (int i = 0; i < roomContainers.size(); i++)
        {
            trigger containerTrigger = roomContainers[i].getTriggerWithoutCommand();
            if (containerTrigger.getActivated() == false)
                {
                condition containerTriggerCondition = containerTrigger.getCondition();
                string object, status, owner, has;
                object = containerTriggerCondition.getObject();
                status = containerTriggerCondition.getStatus();
                owner = containerTriggerCondition.getOwner();
                has = containerTriggerCondition.getHas();
                if (has == "yes")
                    {
                    if (owner == roomContainers[i].getName())
                        {
                        item returnItem = roomContainers[i].checkItems(object);
                        if (returnItem.getName() == object)
                            {
                            containerTrigger.executePrint();
                            containerTrigger.updateActivated();
                            roomContainers[i].removeTriggerWithoutCommand();
                            roomContainers[i].addTrigger(containerTrigger);
                            string action = containerTrigger.getAction();
                            vector<string> actionVect;
                            separateWords(action, actionVect);
                            roomContainers[i].updateStatus(actionVect[actionVect.size()-1]);
                            }
                        }
                    }
                current.removeContainer(roomContainers[i].getName());
                current.addContainer(roomContainers[i]);
                }
        }
}

bool triggersInRoom(room current, string userinput)
{
    vector<string> roomTriggerCommands;
    current.getRoomTriggerCommands(roomTriggerCommands);
    for (int t = 0; t < roomTriggerCommands.size(); t++)
        {
        if (userinput == roomTriggerCommands[t])
            {
            return true;
            }
        }
    return false;
}

bool triggersInContainers(room current, string userinput, container & triggerPresentContainer)
{
    vector<container> roomContainers;
    current.getContainers(roomContainers);
    vector<string> containerTriggerCommands;
    for (int t = 0; t < roomContainers.size(); t++)
        {
            containerTriggerCommands.clear();
            roomContainers[t].getContainerTriggerComands(containerTriggerCommands);
            for (int j = 0; j<containerTriggerCommands.size(); j++)
                {
                    if(userinput == containerTriggerCommands[j])
                        {
                        triggerPresentContainer = roomContainers[t];
                        return true;
                        }
                }
        }
    return false;

}

void activateRoomCommandTriggers(room current, string userinput, player user, bool & triggersPresentInContainers, bool & triggersPresentInRoom)
{
    trigger commandTrigger = current.checkTriggersByCommand(userinput);
    condition triggerCondition = commandTrigger.getCondition();
    string has, object, owner;
    has = triggerCondition.getHas();
    object = triggerCondition.getObject();
    owner = triggerCondition.getOwner();
    if (has == "no")
        {
        if (owner == "inventory")
            {
            item returnItem = user.checkItems(object);
            if (returnItem.getName() == "dummy")
                {
                commandTrigger.executePrint();
                }
            else
                {
                triggersPresentInContainers = false;
                triggersPresentInRoom = false;
                }
            }
        }
    else
        {
        if (owner == "inventory")
            {
            item returnItem = user.checkItems(object);
            if (returnItem.getName() == "dummy")
                {
                triggersPresentInContainers = false;
                triggersPresentInRoom = false;
                }
            else
                {
                commandTrigger.executePrint();
                }
            }
        }
}

void activateContainerCommandTriggers(container triggerPresentContainer, string userinput, bool & triggersPresentInContainers, bool & triggersPresentInRoom)
{
    trigger commandTrigger = triggerPresentContainer.checkTriggersByCommand(userinput);
    condition triggerCondition = commandTrigger.getCondition();
    string status = triggerCondition.getStatus();
    if (triggerPresentContainer.getStatus() == status)
        {
        commandTrigger.executePrint();

        }
    else
        {
        triggersPresentInContainers = false;
        triggersPresentInRoom = false;
        }
}