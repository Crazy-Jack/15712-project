/**
 * @file pbft_nodes.cpp
 * @author your name (you@domain.com)
 * @brief stuff
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "pbft_nodes.h"

/** PBFT Good Nodes */
/** REQUEST STAGE */
void PBFTGoodNode::ReceiveRequestMsg(PBFTMessage& msg){
  
}

/** PRE-PREPARE STAGE */
PBFTMessage PBFTGoodNode::GeneratePrePrepareMsg() {

}

void PBFTGoodNode::ReceivePrePrepareMsg(PBFTMessage& msg) {

}

/** PREPARE STAGE */
PBFTMessage PBFTGoodNode::GeneratePrepareMsg() {

}
void PBFTGoodNode::ReceivePrepareMsg(PBFTMessage& msg) {

}

/** COMMIT STAGE */
PBFTMessage PBFTGoodNode::GenerateCommitMsg() {

}
void PBFTGoodNode::ReceiveCommitMsg(PBFTMessage& msg) {

}


/** PBFT Byzantine Nodes */
/** REQUEST STAGE */
void PBFTByzantineNode::ReceiveRequestMsg(PBFTMessage& msg){

}

/** PRE-PREPARE STAGE */
PBFTMessage PBFTByzantineNode::GeneratePrePrepareMsg() {

}

void PBFTByzantineNode::ReceivePrePrepareMsg(PBFTMessage& msg) {

}

/** PREPARE STAGE */
PBFTMessage PBFTByzantineNode::GeneratePrepareMsg() {

}
void PBFTByzantineNode::ReceivePrepareMsg(PBFTMessage& msg) {

}

/** COMMIT STAGE */
PBFTMessage PBFTByzantineNode::GenerateCommitMsg() {

}
void PBFTByzantineNode::ReceiveCommitMsg(PBFTMessage& msg) {

}