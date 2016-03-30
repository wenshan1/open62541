#include "ua_types.h"
#include "ua_server_internal.h"
#include "ua_nodestore.h"
#include "ua_subscription_manager.h"

void SubscriptionManager_init(UA_Session *session) {
    UA_SubscriptionManager *manager = &(session->subscriptionManager);
    LIST_INIT(&manager->serverSubscriptions);
    manager->lastSessionID = UA_UInt32_random();
}

void SubscriptionManager_deleteMembers(UA_Session *session, UA_Server *server) {
    UA_SubscriptionManager *manager = &session->subscriptionManager;
    UA_Subscription *current, *temp;
    LIST_FOREACH_SAFE(current, &manager->serverSubscriptions, listEntry, temp) {
        LIST_REMOVE(current, listEntry);
        UA_Subscription_deleteMembers(current, server);
        UA_free(current);
    }
}

void SubscriptionManager_addSubscription(UA_SubscriptionManager *manager, UA_Subscription *newSubscription) {
    LIST_INSERT_HEAD(&manager->serverSubscriptions, newSubscription, listEntry);
}

UA_Subscription *
SubscriptionManager_getSubscriptionByID(UA_SubscriptionManager *manager, UA_UInt32 subscriptionID) {
    UA_Subscription *sub;
    LIST_FOREACH(sub, &manager->serverSubscriptions, listEntry) {
        if(sub->subscriptionID == subscriptionID)
            break;
    }
    return sub;
}

UA_StatusCode
SubscriptionManager_deleteMonitoredItem(UA_SubscriptionManager *manager, UA_UInt32 subscriptionID,
                                        UA_UInt32 monitoredItemID) {
    UA_Subscription *sub = SubscriptionManager_getSubscriptionByID(manager, subscriptionID);
    if(!sub)
        return UA_STATUSCODE_BADSUBSCRIPTIONIDINVALID;
    
    UA_MonitoredItem *mon, *tmp_mon;
    LIST_FOREACH_SAFE(mon, &sub->MonitoredItems, listEntry, tmp_mon) {
        if(mon->itemId == monitoredItemID) {
            LIST_REMOVE(mon, listEntry);
            MonitoredItem_delete(mon);
            return UA_STATUSCODE_GOOD;
        }
    }
    return UA_STATUSCODE_BADMONITOREDITEMIDINVALID;
}

UA_StatusCode
SubscriptionManager_deleteSubscription(UA_Server *server, UA_SubscriptionManager *manager,
                                       UA_UInt32 subscriptionID) {
    UA_Subscription *sub = SubscriptionManager_getSubscriptionByID(manager, subscriptionID);    
    if(!sub)
        return UA_STATUSCODE_BADSUBSCRIPTIONIDINVALID;
    LIST_REMOVE(sub, listEntry);
    UA_Subscription_deleteMembers(sub, server);
    UA_free(sub);
    return UA_STATUSCODE_GOOD;
} 

UA_UInt32 SubscriptionManager_getUniqueUIntID(UA_SubscriptionManager *manager) {
    UA_UInt32 id = ++(manager->lastSessionID);
    return id;
}
