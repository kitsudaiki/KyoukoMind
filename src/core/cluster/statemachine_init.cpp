/**
 * @file        statemachine_init.cpp
 *
 * @author      Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 * @copyright   Apache License Version 2.0
 *
 *      Copyright 2019 Tobias Anker
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use cluster file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#include "statemachine_init.h"

#include <core/cluster/states/task_handle_state.h>
#include <core/cluster/states/cycle_finish_state.h>
#include <core/cluster/states/graphs/graph_interpolation_state.h>
#include <core/cluster/states/graphs/graph_learn_backward_state.h>
#include <core/cluster/states/graphs/graph_learn_forward_state.h>
#include <core/cluster/states/images/image_identify_state.h>
#include <core/cluster/states/images/image_learn_backward_state.h>
#include <core/cluster/states/images/image_learn_forward_state.h>
#include <core/cluster/states/snapshots/save_cluster_state.h>
#include <core/cluster/states/snapshots/restore_cluster_state.h>

#include <core/cluster/cluster.h>

#include <libKitsunemimiCommon/statemachine.h>

/**
 * @brief initStates
 * @param sm
 */
void
initStates(Kitsunemimi::Statemachine &sm)
{
    sm.createNewState(TASK_STATE,                       "Task-handling mode");
    sm.createNewState(LEARN_STATE,                      "Learn-State");
    sm.createNewState(IMAGE_LEARN_STATE,                "Image-learn state");
    sm.createNewState(IMAGE_LEARN_FORWARD_STATE,        "Image-learn state: forward-propagation");
    sm.createNewState(IMAGE_LEARN_BACKWARD_STATE,       "Image-learn state: backward-propagation");
    sm.createNewState(IMAGE_LEARN_CYCLE_FINISH_STATE,   "Image-learn state: finish-cycle");
    sm.createNewState(GRAPH_LEARN_STATE,                "Graph-learn state");
    sm.createNewState(GRAPH_LEARN_FORWARD_STATE,        "Graph-learn state: forward-propagation");
    sm.createNewState(GRAPH_LEARN_BACKWARD_STATE,       "Graph-learn state: backward-propagation");
    sm.createNewState(GRAPH_LEARN_CYCLE_FINISH_STATE,   "Graph-learn state: finish-cycle");
    sm.createNewState(REQUEST_STATE,                    "Request-State");
    sm.createNewState(IMAGE_REQUEST_STATE,              "Image-request state");
    sm.createNewState(IMAGE_REQUEST_FORWARD_STATE,      "Image-request state: forward-propagation");
    sm.createNewState(IMAGE_REQUEST_CYCLE_FINISH_STATE, "Image-request state: finish-cycle");
    sm.createNewState(GRAPH_REQUEST_STATE,              "Graph-request state");
    sm.createNewState(GRAPH_REQUEST_FORWARD_STATE,      "Graph-request state: forward-propagation");
    sm.createNewState(GRAPH_REQUEST_CYCLE_FINISH_STATE, "Graph-request state: finish-cycle");
    sm.createNewState(SNAPSHOT_STATE,                   "Snapshot state");
    sm.createNewState(CLUSTER_SNAPSHOT_STATE,           "Cluster-snapshot state");
    sm.createNewState(CLUSTER_SNAPSHOT_SAVE_STATE,      "Cluster-snapshot state: save");
    sm.createNewState(CLUSTER_SNAPSHOT_RESTORE_STATE,   "Cluster-snapshot state: restore");
    sm.createNewState(DIRECT_STATE,                     "Direct mode");
}

/**
 * @brief initEvents
 * @param sm
 * @param cluster
 * @param taskState
 */
void
initEvents(Kitsunemimi::Statemachine &sm,
           Cluster* cluster,
           TaskHandle_State* taskState)
{
    sm.addEventToState(TASK_STATE,                       taskState);
    sm.addEventToState(IMAGE_LEARN_FORWARD_STATE,        new ImageLearnForward_State(cluster));
    sm.addEventToState(IMAGE_LEARN_BACKWARD_STATE,       new ImageLearnBackward_State(cluster));
    sm.addEventToState(GRAPH_LEARN_FORWARD_STATE,        new GraphLearnForward_State(cluster));
    sm.addEventToState(GRAPH_LEARN_BACKWARD_STATE,       new GraphLearnBackward_State(cluster));
    sm.addEventToState(IMAGE_REQUEST_FORWARD_STATE,      new ImageIdentify_State(cluster));
    sm.addEventToState(GRAPH_REQUEST_FORWARD_STATE,      new GraphInterpolation_State(cluster));
    sm.addEventToState(IMAGE_LEARN_CYCLE_FINISH_STATE,   new CycleFinish_State(cluster));
    sm.addEventToState(GRAPH_LEARN_CYCLE_FINISH_STATE,   new CycleFinish_State(cluster));
    sm.addEventToState(IMAGE_REQUEST_CYCLE_FINISH_STATE, new CycleFinish_State(cluster));
    sm.addEventToState(GRAPH_REQUEST_CYCLE_FINISH_STATE, new CycleFinish_State(cluster));
    sm.addEventToState(CLUSTER_SNAPSHOT_SAVE_STATE,      new SaveCluster_State(cluster));
    sm.addEventToState(CLUSTER_SNAPSHOT_RESTORE_STATE,   new RestoreCluster_State(cluster));
}

/**
 * @brief initChildStates
 * @param sm
 */
void
initChildStates(Kitsunemimi::Statemachine &sm)
{
    // child states image learn
    sm.addChildState(LEARN_STATE,       IMAGE_LEARN_STATE);
    sm.addChildState(IMAGE_LEARN_STATE, IMAGE_LEARN_FORWARD_STATE);
    sm.addChildState(IMAGE_LEARN_STATE, IMAGE_LEARN_BACKWARD_STATE);
    sm.addChildState(IMAGE_LEARN_STATE, IMAGE_LEARN_CYCLE_FINISH_STATE);

    // child states graph learn
    sm.addChildState(LEARN_STATE,       GRAPH_LEARN_STATE);
    sm.addChildState(GRAPH_LEARN_STATE, GRAPH_LEARN_FORWARD_STATE);
    sm.addChildState(GRAPH_LEARN_STATE, GRAPH_LEARN_BACKWARD_STATE);
    sm.addChildState(GRAPH_LEARN_STATE, GRAPH_LEARN_CYCLE_FINISH_STATE);

    // child states image request
    sm.addChildState(REQUEST_STATE,       IMAGE_REQUEST_STATE);
    sm.addChildState(IMAGE_REQUEST_STATE, IMAGE_REQUEST_FORWARD_STATE);
    sm.addChildState(IMAGE_REQUEST_STATE, IMAGE_REQUEST_CYCLE_FINISH_STATE);

    // child states graph request
    sm.addChildState(REQUEST_STATE,       GRAPH_REQUEST_STATE);
    sm.addChildState(GRAPH_REQUEST_STATE, GRAPH_REQUEST_FORWARD_STATE);
    sm.addChildState(GRAPH_REQUEST_STATE, GRAPH_REQUEST_CYCLE_FINISH_STATE);

    // child states snapshot
    sm.addChildState(SNAPSHOT_STATE,         CLUSTER_SNAPSHOT_STATE);
    sm.addChildState(CLUSTER_SNAPSHOT_STATE, CLUSTER_SNAPSHOT_SAVE_STATE);
    sm.addChildState(CLUSTER_SNAPSHOT_STATE, CLUSTER_SNAPSHOT_RESTORE_STATE);
}

/**
 * @brief initInitialChildStates
 * @param sm
 */
void
initInitialChildStates(Kitsunemimi::Statemachine &sm)
{
    sm.setInitialChildState(IMAGE_LEARN_STATE,   IMAGE_LEARN_FORWARD_STATE);
    sm.setInitialChildState(GRAPH_LEARN_STATE,   GRAPH_LEARN_FORWARD_STATE);
    sm.setInitialChildState(IMAGE_REQUEST_STATE, IMAGE_REQUEST_FORWARD_STATE);
    sm.setInitialChildState(GRAPH_REQUEST_STATE, GRAPH_REQUEST_FORWARD_STATE);
}

/**
 * @brief initTransitions
 * @param sm
 */
void
initTransitions(Kitsunemimi::Statemachine &sm)
{

    // transtions learn init
    sm.addTransition(TASK_STATE,  LEARN, LEARN_STATE);
    sm.addTransition(LEARN_STATE, IMAGE, IMAGE_LEARN_STATE);
    sm.addTransition(LEARN_STATE, GRAPH, GRAPH_LEARN_STATE);

    // transitions request init
    sm.addTransition(TASK_STATE,    REQUEST, REQUEST_STATE);
    sm.addTransition(REQUEST_STATE, IMAGE,   IMAGE_REQUEST_STATE);
    sm.addTransition(REQUEST_STATE, GRAPH,   GRAPH_REQUEST_STATE);

    // transitions snapshot init
    sm.addTransition(TASK_STATE,             SNAPSHOT, SNAPSHOT_STATE);
    sm.addTransition(SNAPSHOT_STATE,         CLUSTER,  CLUSTER_SNAPSHOT_STATE);
    sm.addTransition(CLUSTER_SNAPSHOT_STATE, SAVE,     CLUSTER_SNAPSHOT_SAVE_STATE);
    sm.addTransition(CLUSTER_SNAPSHOT_STATE, RESTORE,  CLUSTER_SNAPSHOT_RESTORE_STATE);

    // trainsition learn-internal
    sm.addTransition(IMAGE_LEARN_FORWARD_STATE,      NEXT, IMAGE_LEARN_BACKWARD_STATE     );
    sm.addTransition(IMAGE_LEARN_BACKWARD_STATE,     NEXT, IMAGE_LEARN_CYCLE_FINISH_STATE );
    sm.addTransition(IMAGE_LEARN_CYCLE_FINISH_STATE, NEXT, IMAGE_LEARN_FORWARD_STATE      );
    sm.addTransition(GRAPH_LEARN_FORWARD_STATE,      NEXT, GRAPH_LEARN_BACKWARD_STATE     );
    sm.addTransition(GRAPH_LEARN_BACKWARD_STATE,     NEXT, GRAPH_LEARN_CYCLE_FINISH_STATE );
    sm.addTransition(GRAPH_LEARN_CYCLE_FINISH_STATE, NEXT, GRAPH_LEARN_FORWARD_STATE      );

    // trainsition request-internal
    sm.addTransition(IMAGE_REQUEST_FORWARD_STATE,      NEXT, IMAGE_REQUEST_CYCLE_FINISH_STATE );
    sm.addTransition(IMAGE_REQUEST_CYCLE_FINISH_STATE, NEXT, IMAGE_REQUEST_FORWARD_STATE      );
    sm.addTransition(GRAPH_REQUEST_FORWARD_STATE,      NEXT, GRAPH_REQUEST_CYCLE_FINISH_STATE );
    sm.addTransition(GRAPH_REQUEST_CYCLE_FINISH_STATE, NEXT, GRAPH_REQUEST_FORWARD_STATE      );

    // transition finish back to task-state
    sm.addTransition(LEARN_STATE,                    FINISH_TASK, TASK_STATE);
    sm.addTransition(REQUEST_STATE,                  FINISH_TASK, TASK_STATE);
    sm.addTransition(SNAPSHOT_STATE,                 FINISH_TASK, TASK_STATE);
    sm.addTransition(CLUSTER_SNAPSHOT_SAVE_STATE,    FINISH_TASK, TASK_STATE);
    sm.addTransition(CLUSTER_SNAPSHOT_RESTORE_STATE, FINISH_TASK, TASK_STATE);

    // special transition to tigger the task-state again
    sm.addTransition(TASK_STATE, PROCESS_TASK, TASK_STATE);

    // mode-switches
    sm.addTransition(TASK_STATE, SWITCH_TO_DIRECT_MODE, DIRECT_STATE);
    sm.addTransition(DIRECT_STATE, SWITCH_TO_TASK_MODE, TASK_STATE);
}

/**
 * @brief initStatemachine
 * @param sm
 * @param cluster
 * @param taskState
 */
void
initStatemachine(Kitsunemimi::Statemachine &sm,
                 Cluster* cluster,
                 TaskHandle_State* taskState)
{
    initStates(sm);
    initEvents(sm, cluster, taskState);
    initChildStates(sm);
    initInitialChildStates(sm);
    initTransitions(sm);

    // set initial state for the state-machine
    sm.setCurrentState(TASK_STATE);
}
