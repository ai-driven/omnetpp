//=========================================================================
//  EVENT.CC - part of
//                  OMNeT++/OMNEST
//           Discrete System Simulation in C++
//
//=========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 1992-2006 Andras Varga

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#include "linetokenizer.h"
#include "filereader.h"
#include "event.h"
#include "eventlog.h"
#include "eventlogentry.h"

MessageSend::MessageSend(EventLog *eventLog, long senderEventNumber, int messageSendEntryNumber)
{
    this->eventLog = eventLog;
    this->senderEventNumber = senderEventNumber;
    this->messageSendEntryNumber = messageSendEntryNumber;
    arrivalEventNumber = -1;
}

Event *MessageSend::getSenderEvent()
{
    return eventLog->getEvent(senderEventNumber);
}

EventLogEntry *MessageSend::getMessageSendEntry()
{
    Event *event = getSenderEvent();
    return event->getEventLogEntry(messageSendEntryNumber);
}

bool MessageSend::isMessageSend(EventLogEntry *eventLogEntry)
{
    return
        dynamic_cast<BeginSendEntry *>(eventLogEntry) != NULL ||
        dynamic_cast<MessageScheduledEntry *>(eventLogEntry) != NULL;
}

long MessageSend::getMessageId()
{
    return MessageSend::getMessageId(getMessageSendEntry());
}

long MessageSend::getMessageId(EventLogEntry *eventLogEntry)
{
    // 1. BeginSendEntry
    BeginSendEntry *beginSendEntry = dynamic_cast<BeginSendEntry *>(eventLogEntry);

    if (beginSendEntry != NULL)
        return beginSendEntry->messageId;

    // 2. MessageScheduledEntry
    MessageScheduledEntry *messageScheduledEntry = dynamic_cast<MessageScheduledEntry *>(eventLogEntry);

    if (messageScheduledEntry != NULL)
        return messageScheduledEntry->messageId;

    throw new Exception("Unknown message entry");
}

long MessageSend::getArrivalEventNumber() 
{
    if (arrivalEventNumber == -1)
    {
        simtime_t arrivalTime = getArrivalTime();
        long offset = eventLog->getOffsetForSimulationTime(arrivalTime, true);

        while (true)
        {
            Event *event = eventLog->getEventForOffset(offset);

            if (event->getMessageId() == getMessageId())
            {
                arrivalEventNumber = event->getEventNumber();
                break;
            }

            if (event->getSimulationTime() > arrivalTime)
            {
                throw new Exception("Could not find arrival event");
            }

            offset = event->getEndOffset();
        }
    }

    if (arrivalEventNumber <= senderEventNumber)
        throw new Exception("Invalid message send");

    return arrivalEventNumber;
}

simtime_t MessageSend::getArrivalTime()
{
    Event *event = getSenderEvent();
    EventLogEntry *eventLogEntry = event->getEventLogEntry(messageSendEntryNumber);

    // 1. BeginSendEntry
    BeginSendEntry *beginSendEntry = dynamic_cast<BeginSendEntry *>(eventLogEntry);

    if (beginSendEntry != NULL)
    {
        int i = messageSendEntryNumber;
        simtime_t arrivalTime;

        while (true)
        {
            eventLogEntry = event->getEventLogEntry(i);
            MessageSendHopEntry *messageSendHopEntry = dynamic_cast<MessageSendHopEntry *>(eventLogEntry);

            if (messageSendHopEntry != NULL)
                arrivalTime = event->getSimulationTime() + messageSendHopEntry->propagationDelay;
            else
                break;
        }

        return arrivalTime;
    }

    // 2. MessageScheduledEntry
    MessageScheduledEntry *messageScheduledEntry = dynamic_cast<MessageScheduledEntry *>(eventLogEntry);

    if (messageScheduledEntry != NULL)
        return messageScheduledEntry->arrivalTime;

    throw new Exception("Unknown message entry");
}

Event *MessageSend::getArrivalEvent() 
{
    return eventLog->getEvent(getArrivalEventNumber());
}

/**************************************************/

long Event::numParseEvent = 0;

Event::Event(EventLog *eventLog)
{
    this->eventLog = eventLog;
    beginOffset = -1;
    endOffset = -1;
    eventEntry = NULL;
    cause = NULL;
    causes = NULL;
    consequences = NULL;
}

Event::~Event()
{
    for (EventLogEntryList::iterator it = eventLogEntries.begin(); it != eventLogEntries.end(); it++)
    {
        delete *it;
    }
}

Event *Event::getCauseEvent()
{
    if (getCauseEventNumber() != -1)
        return eventLog->getEvent(getCauseEventNumber());
    else
        return NULL;
}

MessageSend *Event::getCause()
{
    if (cause == NULL)
    {
        Event *event = getCauseEvent();

        if (event != NULL)
        {
            for (int messageEntryNumber = 0; messageEntryNumber < event->eventLogEntries.size(); messageEntryNumber++)
            {
                EventLogEntry *eventLogEntry = event->eventLogEntries[messageEntryNumber];

                if (MessageSend::isMessageSend(eventLogEntry) &&
                    MessageSend::getMessageId(eventLogEntry) == getMessageId())
                {
                    cause = new MessageSend(eventLog, getCauseEventNumber(), messageEntryNumber);
                    break;
                }
            }
        }
    }

    return cause;
}

Event::MessageSendList *Event::getCauses()
{
    if (causes == NULL)
    {
        causes = new MessageSendList();

        if (getCause() != NULL)
            causes->push_back(getCause());

        // TODO: add "reuse" message send causes
    }

    return causes;
}

Event::MessageSendList *Event::getConsequences()
{
    if (consequences == NULL)
    {
        consequences = new MessageSendList();

        for (int messageEntryNumber = 0; messageEntryNumber < eventLogEntries.size(); messageEntryNumber++)
        {
            EventLogEntry *eventLogEntry = eventLogEntries[messageEntryNumber];

            if (MessageSend::isMessageSend(eventLogEntry))
                consequences->push_back(new MessageSend(eventLog, getEventNumber(), messageEntryNumber));
        }
    }

    return consequences;
}

long Event::parse(FileReader *reader, long offset)
{
    if (eventEntry != NULL)
        throw new Exception("Reusing event objects are not supported");

    beginOffset = offset;
    numParseEvent++;
    reader->seekTo(offset);

    while (true)
    {
        char *line = reader->readLine();

        if (!line)
            return reader->fileSize();

        EventLogEntry *eventLogEntry = EventLogEntry::parseEntry(line);
        EventEntry *eventEntry = dynamic_cast<EventEntry *>(eventLogEntry);

        if (eventEntry)
        {
            if (this->eventEntry)
                break; // stop at the start of next event
            else
                this->eventEntry = eventEntry;
        }

        if (eventLogEntry)
            eventLogEntries.push_back(eventLogEntry);
    }

    return endOffset = reader->lineStartOffset();
}

void Event::print(FILE *file)
{
    for (EventLogEntryList::iterator it = eventLogEntries.begin(); it != eventLogEntries.end(); it++)
    {
        EventLogEntry *eventLogEntry = *it;
        eventLogEntry->print(file);
    }
}
