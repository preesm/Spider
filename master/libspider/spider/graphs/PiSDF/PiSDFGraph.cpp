/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2014 - 2019) :
 *
 * Antoine Morvan <antoine.morvan@insa-rennes.fr> (2018 - 2019)
 * Florian Arrestier <florian.arrestier@insa-rennes.fr> (2018 - 2019)
 * Hugo Miomandre <hugo.miomandre@insa-rennes.fr> (2017)
 * Julien Heulot <julien.heulot@insa-rennes.fr> (2014 - 2016)
 *
 * Spider is a dataflow based runtime used to execute dynamic PiSDF
 * applications. The Preesm tool may be used to design PiSDF applications.
 *
 * This software is governed by the CeCILL  license under French law and
 * abiding by the rules of distribution of free software.  You can  use,
 * modify and/ or redistribute the software under the terms of the CeCILL
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and  rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty  and the software's author,  the holder of the
 * economic rights,  and the successive licensors  have only  limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading,  using,  modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean  that it is complicated to manipulate,  and  that  also
 * therefore means  that it is reserved for developers  and  experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and,  more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL license and that you accept its terms.
 */
#include <graphs/PiSDF/PiSDFCommon.h>
#include <graphs/PiSDF/PiSDFEdge.h>

PiSDFGraph::PiSDFGraph(
        int nEdges, int nParams,
        int nInputIf, int nOutputIf,
        int nConfig, int nBody) :

        edges_(nEdges + nInputIf + nOutputIf, PISDF_STACK),
        params_(nParams, PISDF_STACK),
        bodies_(nBody + nInputIf + nOutputIf, PISDF_STACK),
        configs_(nConfig, PISDF_STACK),
        inputIfs_(nInputIf, PISDF_STACK),
        outputIfs_(nOutputIf, PISDF_STACK) {
    parent_ = nullptr;
    isStatic_ = false;
}

PiSDFGraph::~PiSDFGraph() {
    while (edges_.size() > 0)
        delEdge(edges_[0]);
    while (bodies_.size() > 0) {
        if (bodies_[0]->isHierarchical()) {
            bodies_[0]->getSubGraph()->~PiSDFGraph();
            StackMonitor::free(PISDF_STACK, bodies_[0]->getSubGraph());
        }
        delVertex(bodies_[0]);
    }
    while (configs_.size() > 0)
        delVertex(configs_[0]);
    while (inputIfs_.size() > 0)
        delVertex(inputIfs_[0]);
    while (outputIfs_.size() > 0)
        delVertex(outputIfs_[0]);
    while (params_.size() > 0)
        delParam(params_[0]);
}

PiSDFVertex *PiSDFGraph::addBodyVertex(
        const char *vertexName, int fctId,
        int nInEdge, int nOutEdge,
        int nInParam) {
    auto *body = CREATE(PISDF_STACK, PiSDFVertex)(
            vertexName, fctId,
            bodies_.size(),
            PISDF_TYPE_BODY, PISDF_SUBTYPE_NORMAL,
            this, nullptr,
            nInEdge, nOutEdge,
            nInParam, 0);
    bodies_.add(body);
    return body;
}

PiSDFVertex *PiSDFGraph::addBodyVertex(
        const char *vertexName, PiSDFSubType type,
        int nInEdge, int nOutEdge,
        int nInParam) {
    auto *body = CREATE(PISDF_STACK, PiSDFVertex)(
            vertexName, -1,
            bodies_.size(),
            PISDF_TYPE_BODY, type,
            this, nullptr,
            nInEdge, nOutEdge,
            nInParam, 0);
    bodies_.add(body);
    return body;
}

PiSDFVertex *PiSDFGraph::addSpecialVertex(
        PiSDFSubType type,
        int nInEdge, int nOutEdge,
        int nInParam) {
    auto *body = CREATE(PISDF_STACK, PiSDFVertex)(
            nullptr, -1,
            bodies_.size(),
            PISDF_TYPE_BODY, type,
            this, nullptr,
            nInEdge, nOutEdge,
            nInParam, 0);
    bodies_.add(body);
    return body;
}

PiSDFVertex *PiSDFGraph::addConfigVertex(
        const char *vertexName, int fctId,
        PiSDFSubType subType,
        int nInEdge, int nOutEdge,
        int nInParam, int nOutParam) {
    auto *config = CREATE(PISDF_STACK, PiSDFVertex)(
            vertexName, fctId,
            configs_.size(),
            PISDF_TYPE_CONFIG, subType,
            this, nullptr,
            nInEdge, nOutEdge,
            nInParam, nOutParam);
    configs_.add(config);
    return config;

}

PiSDFVertex *PiSDFGraph::addInputIf(
        const char *name,
        int nInParam) {
    auto *inIf = CREATE(PISDF_STACK, PiSDFVertex)(
            name, -1,
            inputIfs_.size(),
            PISDF_TYPE_IF, PISDF_SUBTYPE_INPUT_IF,
            this, nullptr,
            0, 1,
            nInParam, 0);
    inputIfs_.add(inIf);
    return inIf;
}

PiSDFVertex *PiSDFGraph::addOutputIf(
        const char *name,
        int nInParam) {
    auto *outIf = CREATE(PISDF_STACK, PiSDFVertex)(
            name, -1,
            outputIfs_.size(),
            PISDF_TYPE_IF, PISDF_SUBTYPE_OUTPUT_IF,
            this, nullptr,
            1, 0,
            nInParam, 0);
    outputIfs_.add(outIf);
    return outIf;
}

PiSDFEdge *PiSDFGraph::addEdge() {
    auto *edge = CREATE(PISDF_STACK, PiSDFEdge)(this);
    edges_.add(edge);
    return edge;
}

PiSDFEdge *PiSDFGraph::connect(
        PiSDFVertex *src, int srcPortId, const char *prod,
        PiSDFVertex *snk, int snkPortId, const char *cons,
        const char *delay,
        PiSDFVertex *setter,
        PiSDFVertex *getter,
        bool isDelayPersistent) {
    PiSDFEdge *edge = this->addEdge();
    edge->connectSrc(src, srcPortId, prod);
    edge->connectSnk(snk, snkPortId, cons);
    edge->setDelay(delay, setter, getter, isDelayPersistent);
    src->connectOutEdge(srcPortId, edge);
    snk->connectInEdge(snkPortId, edge);
    return edge;
}


void PiSDFGraph::delVertex(PiSDFVertex *vertex) {
    switch (vertex->getType()) {
        case PISDF_TYPE_BODY:
            bodies_.del(vertex);
            break;
        case PISDF_TYPE_CONFIG:
            configs_.del(vertex);
            break;
        case PISDF_TYPE_IF:
            if (vertex->getSubType() == PISDF_SUBTYPE_INPUT_IF)
                inputIfs_.del(vertex);
            else
                outputIfs_.del(vertex);
            break;
    }
    vertex->~PiSDFVertex();
    StackMonitor::free(PISDF_STACK, vertex);
}

void PiSDFGraph::delParam(PiSDFParam *param) {
    params_.del(param);
    param->~PiSDFParam();
    StackMonitor::free(PISDF_STACK, param);
}

void PiSDFGraph::delEdge(PiSDFEdge *edge) {
    edges_.del(edge);
    edge->~PiSDFEdge();
    StackMonitor::free(PISDF_STACK, edge);
}

/** Print Fct */
void PiSDFGraph::print(const char *path) {
    FILE *file = Platform::get()->fopen(path);
    if (file == nullptr) {
        throwSpiderException("failed to open file: %s", path);
    }

    // Writing header
    Platform::get()->fprintf(file, "digraph csdag {\n");
    Platform::get()->fprintf(file, "\tnode [color=\"#433D63\"];\n");
    Platform::get()->fprintf(file, "\tedge [color=\"#9262B6\" arrowhead=\"empty\"];\n");
    Platform::get()->fprintf(file, "\trankdir=LR;\n\n");


    // Drawing parameters.
    Platform::get()->fprintf(file, "\t# Parameters\n");
    for (int i = 0; i < params_.size(); i++) {
        PiSDFParam *param = params_[i];
        Platform::get()->fprintf(file, "\t%s [label=\"%s\" shape=house];\n",
                                 param->getName(),
                                 param->getName());
    }

    // Drawing Config PiSDF vertices.
    Platform::get()->fprintf(file, "\n\t# Configs\n");
    for (int i = 0; i < configs_.size(); i++) {
        PiSDFVertex *config = configs_[i];
        Platform::get()->fprintf(file, "\t%d [shape=doubleoctagon,label=\"%s\"];\n",
                                 config->getId(),
                                 config->getName());

        // Drawing lines : vertex -> parameters.
        for (int j = 0; j < config->getNOutParam(); j++) {
            Platform::get()->fprintf(file, "\t%d->%s [style=dotted];\n",
                                     config->getId(),
                                     config->getOutParam(j)->getName());
        }

        // Drawing lines : parameter -> vertex.
        for (int j = 0; j < config->getNInParam(); j++) {
            Platform::get()->fprintf(file, "\t%s->%d [style=dotted];\n",
                                     config->getInParam(j)->getName(),
                                     config->getId());
        }
        Platform::get()->fprintf(file, "\n");
    }

    // Drawing Body PiSDF vertices.
    Platform::get()->fprintf(file, "\t# Body Vertices\n");
    for (int i = 0; i < bodies_.size(); i++) {
        PiSDFVertex *body = bodies_[i];
        if (body->isHierarchical()) {
            char name[100];
            sprintf(name, "%s_sub.gv", body->getName());
            body->getSubGraph()->print(name);
        }

        Platform::get()->fprintf(file, "\t%d [label=\"%s\"];\n",
                                 body->getId(),
                                 body->getName());

        // Drawing lines : parameter -> vertex.
        for (int j = 0; j < body->getNInParam(); j++) {
            Platform::get()->fprintf(file, "\t%s->%d [style=dotted];\n",
                                     body->getInParam(j)->getName(),
                                     body->getId());
        }
        Platform::get()->fprintf(file, "\n");
    }

    // Drawing Input vertices.
    Platform::get()->fprintf(file, "\t# Input Ifs\n");
    for (int i = 0; i < inputIfs_.size(); i++) {
        PiSDFVertex *inIf = inputIfs_[i];
        Platform::get()->fprintf(file, "\t%d [shape=cds,label=\"%s\"];\n",
                                 inIf->getId(),
                                 inIf->getName());

        // Drawing lines : parameter -> vertex.
        for (int j = 0; j < inIf->getNInParam(); j++) {
            Platform::get()->fprintf(file, "\t%s->%d [style=dotted];\n",
                                     inIf->getInParam(j)->getName(),
                                     inIf->getId());
        }
        Platform::get()->fprintf(file, "\n");
    }

    // Drawing Output vertices.
    Platform::get()->fprintf(file, "\t# Output Ifs\n");
    for (int i = 0; i < outputIfs_.size(); i++) {
        PiSDFVertex *outIf = outputIfs_[i];
        Platform::get()->fprintf(file, "\t%d [shape=cds,label=\"%s\"];\n",
                                 outIf->getId(),
                                 outIf->getName());

        // Drawing lines : parameter -> vertex.
        for (int j = 0; j < outIf->getNInParam(); j++) {
            Platform::get()->fprintf(file, "\t%s->%d [style=dotted];\n",
                                     outIf->getInParam(j)->getName(),
                                     outIf->getId());
        }
        Platform::get()->fprintf(file, "\n");
    }

    // Drawing edges.
    Platform::get()->fprintf(file, "\t# Edges\n");
    for (int i = 0; i < edges_.size(); i++) {
        PiSDFEdge *edge = edges_[i];
        char prodExpr[100];
        char consExpr[100];
        char delayExpr[100];

        edge->getProdExpr(prodExpr, 100);
        edge->getConsExpr(consExpr, 100);
        edge->getDelayExpr(delayExpr, 100);

//		Parser_toString(&(edge->production), &(graph->params), shortenedPExpr);
//		Parser_toString(&(edge->consumption), &(graph->params), shortenedCExpr);

        /*Platform::get()->fprintf("\t%s->%s [taillabel=\"%s\" headlabel=\"%s\" labeldistance=%d labelangle=50];\n",
            edge->getSource()->getName(),edge->getSink()->getName(),
            shortenedPExpr,shortenedCExpr,labelDistance);*/
//		Platform::get()->fprintf(file, "\t%s->%s [taillabel=\"(%d):%s\" headlabel=\"(%d):%s\"];\n",
//			edge->source->name,
//			edge->sink->name,
//			edge->sourcePortIx,
//			shortenedPExpr,
//			edge->sinkPortIx,
//			shortenedCExpr);
        //labelDistance = 3 + labelDistance%(3*4); // Oscillating the label distance to keep visibility
        Platform::get()->fprintf(file, "\t%d->%d [taillabel=\"(%d):%s\" headlabel=\"(%d):%s\" label=\"%s\"];\n",
                                 edge->getSrc()->getId(),
                                 edge->getSnk()->getId(),
                                 edge->getSrcPortIx(),
                                 prodExpr,
                                 edge->getSnkPortIx(),
                                 consExpr,
                                 delayExpr);
    }

    Platform::get()->fprintf(file, "}\n");
    Platform::get()->fclose(file);
}
