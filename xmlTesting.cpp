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



int main (int argc, char ** argv) {

    string gameFile = "testGame.xml";

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
        //cout << "At the onset, items are " << endl;
        //current.readItems();
        //used to read creature triggers(activated by status i
        vector<creature> roomCreatures;
        current.getCreatures(roomCreatures);
        for (int i = 0; i < roomCreatures.size(); i++)
            {
                trigger creatureTrigger = roomCreatures[i].getTrigger();
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
                                }
                            }
                    }
            }

        string userinput;
        getline(cin, userinput);
        vector<string> inputVect;
        separateWords(userinput, inputVect);



        //all this is to activate triggers that occur because of user commands.
        //for the room, we check if the user has a certain item or not
        //for the containers, we check the status of said container (e.g. locks on a door or whatnot)
        vector<string> roomTriggerCommands;
        bool triggersPresentInRoom = false;
        current.getRoomTriggerCommands(roomTriggerCommands);
        for (int t = 0; t < roomTriggerCommands.size(); t++)
            {
            if (userinput == roomTriggerCommands[t])
                {
                triggersPresentInRoom = true;
                }
            }

        bool triggersPresentInContainers = false;
        vector<container> roomContainers;
        current.getContainers(roomContainers);
        vector<string> containerTriggerCommands;
        container triggerPresentContainer;
        for (int t = 0; t < roomContainers.size(); t++)
            {
                containerTriggerCommands.clear();
                roomContainers[t].getContainerTriggerComands(containerTriggerCommands);
                for (int j = 0; j<containerTriggerCommands.size(); j++)
                    {
                        if(userinput == containerTriggerCommands[j])
                            {
                            triggerPresentContainer = roomContainers[t];
                            triggersPresentInContainers = true;
                            }
                    }
            }

        if ((triggersPresentInRoom) && !(triggersPresentInContainers))
            {
                trigger commandTrigger = current.checkTriggersByCommand(userinput);
                condition triggerCondition = commandTrigger.getCondition();
                string has, object, owner;
                has = triggerCondition.getHas();
                object = triggerCondition.getObject();
                owner = triggerCondition.getOwner();
                if (has == "") //one kind of room trigger
                    {

                    }
                else if (has!= "") //other kind of room trigger. checks user inventory
                    {
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
            }

        else if ((triggersPresentInContainers) && !(triggersPresentInRoom))
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
        //end of room and container triggers (activated by user commands)

        if ( !(triggersPresentInContainers) && !(triggersPresentInRoom))
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
                        }
                }
        if (changedRooms == false)
            {
            user.updateLocation(current);
            }



        }
    return 0;
}



