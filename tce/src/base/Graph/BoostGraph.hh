/*
    Copyright 2002-2008 Tampere University of Technology.  All Rights
    Reserved.

    This file is part of TTA-Based Codesign Environment (TCE).

    TCE is free software; you can redistribute it and/or modify it under the
    terms of the GNU General Public License version 2 as published by the Free
    Software Foundation.

    TCE is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
    details.

    You should have received a copy of the GNU General Public License along
    with TCE; if not, write to the Free Software Foundation, Inc., 51 Franklin
    St, Fifth Floor, Boston, MA  02110-1301  USA

    As a special exception, you may use this file as part of a free software
    library without restriction.  Specifically, if other files instantiate
    templates or use macros or inline functions from this file, or you compile
    this file and link it with other files to produce an executable, this
    file does not by itself cause the resulting executable to be covered by
    the GNU General Public License.  This exception does not however
    invalidate any other reasons why the executable file might be covered by
    the GNU General Public License.
*/
/**
 * @file BoostGraph.hh
 *
 * Declaration of boost-based templated implementation of graph base class.
 *
 * @author Andrea Cilio 2005 (cilio-no.spam-cs.tut.fi)
 * @author Vladimir Guzma 2006 (vladimir.guzma-no.spam-tut.fi)
 * @note rating: red
 */


#ifndef TTA_BOOST_GRAPH_HH
#define TTA_BOOST_GRAPH_HH

#include <map>
#include <set>
#include <boost/config.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/subgraph.hpp>

#include "hash_map.hh"

#include "Exception.hh"
#include "Graph.hh"

/**
 * Graph-based program representation.
 *
 * Boost::graph based implementation. 
 */
template <typename GraphNode, typename GraphEdge>
class BoostGraph : public GraphBase<GraphNode, GraphEdge> {
public:

    typedef std::set<GraphNode*, typename GraphNode::Comparator > NodeSet;
    typedef std::set<GraphEdge*, typename GraphEdge::Comparator > EdgeSet;

    /// The (base) node type managed by this graph. 
    /// @todo What's the point of these typedefs?
    typedef GraphNode Node;
    /// The (base) edge type managed by this graph.
    typedef GraphEdge Edge;

    BoostGraph();
    BoostGraph(const std::string& name);
    BoostGraph(const BoostGraph& other);

    ~BoostGraph();

    int nodeCount() const;
    int edgeCount() const;
    Node& node(const int index) const
        throw (OutOfRange);
    Node& node(const int index, bool cacheResult) const
        throw (OutOfRange);
    virtual Edge& edge(const int index) const
        throw (OutOfRange);

    virtual void addNode(Node& node)
        throw (ObjectAlreadyExists);

    virtual Edge& outEdge(const Node& node, const int index) const
        throw (OutOfRange, InstanceNotFound);

    virtual Edge& inEdge(const Node& node, const int index) const
        throw (OutOfRange, InstanceNotFound);

    virtual EdgeSet outEdges(const Node& node) const
        throw (InstanceNotFound);
    virtual EdgeSet inEdges(const Node& node) const
        throw (InstanceNotFound);

    virtual EdgeSet rootGraphOutEdges(const Node& node) const
        throw (InstanceNotFound);

    virtual EdgeSet rootGraphInEdges(const Node& node) const
        throw (InstanceNotFound);

    virtual int rootGraphInDegree(const Node& node) const
        throw (InstanceNotFound);

    virtual int rootGraphOutDegree(const Node& node) const
        throw (InstanceNotFound);

    virtual int outDegree(const Node& node) const
        throw (InstanceNotFound);
    virtual int inDegree(const Node& node) const
        throw (InstanceNotFound);

    virtual Node& tailNode(const Edge& edge) const
        throw (InstanceNotFound);
    virtual Node& headNode(const Edge& edge) const
        throw (InstanceNotFound);

    virtual void connectNodes(
        const Node& nTail, const Node& nHead, Edge& e)
        throw (ObjectAlreadyExists);

    virtual void disconnectNodes(const Node& nTail, const Node& nHead);

    virtual void moveInEdges(const Node& source, const Node& destination)
        throw (NotAvailable);
    virtual void moveOutEdges(const Node& source, const Node& destination)
        throw (NotAvailable);

    virtual void removeNode(Node& node)
        throw (InstanceNotFound);
    virtual void removeEdge(Edge& e)
        throw (InstanceNotFound);

    virtual void dropNode(Node& node)
        throw (InstanceNotFound);

    virtual void dropEdge(Edge& edge)
        throw (InstanceNotFound);

    virtual bool hasEdge(
        const Node& nTail,
        const Node& nHead) const;

    /// useful utility functions 
    virtual NodeSet rootNodes() const;

    virtual NodeSet successors(const Node& node) const;
    virtual NodeSet predecessors(const Node& node) const;

    // critical path-related functions
    int maxPathLength(const GraphNode& node) const;
    int maxSinkDistance(const GraphNode& node) const;
    int maxSourceDistance(const GraphNode& node) const;
    int height() const;

    void detachSubgraph(BoostGraph& subGraph);

    BoostGraph* parentGraph();

    bool hasNode(const Node&) const;

    virtual const std::string& name() const;

private:
    /// Assignment forbidden.
    BoostGraph& operator=(const BoostGraph&);

protected:
    /// Internal graph type, providing actual graph-like operations.
    /// This type definition relies on bundled properties of boost library,
    /// which need the host compiler to support partial template
    /// specialisation.

    class GraphHashFunctions {
    public:
        size_t operator()(const GraphNode* node) const {
            int tmp = reinterpret_cast<size_t>(node);
            return tmp ^ (tmp >> 16);
        }
        size_t operator()(const GraphEdge* edge) const {
            int tmp = reinterpret_cast<size_t>(edge);
            return tmp ^ (tmp >> 16);
        }
    };
    
    typedef typename boost::adjacency_list<
        boost::listS, boost::vecS,
        boost::bidirectionalS, Node*, Edge* > Graph;
    
    /// Traits characterising the internal graph type.
    typedef typename boost::graph_traits<Graph> GraphTraits;
    
    /// Output edge iterator type.
    typedef typename GraphTraits::out_edge_iterator OutEdgeIter;
    /// Input edge iterator type.
    typedef typename GraphTraits::in_edge_iterator InEdgeIter;
    /// Iterator type for the list of all edges in the graph.
    typedef typename GraphTraits::edge_iterator EdgeIter;
    /// Iterator type for the list of all nodes in the graph.
    typedef typename GraphTraits::vertex_iterator NodeIter;
    
    /// Type with which edges of the graph are seen internally.
    typedef typename GraphTraits::edge_descriptor EdgeDescriptor;
    /// Type with which nodes of the graph are seen internally.
    typedef typename GraphTraits::vertex_descriptor NodeDescriptor;
    
    // private helper methods

    // this is a slow(linear to size of the graph) operation
    EdgeDescriptor descriptor(const Edge& e) const;
    // these two are much faster operations
    EdgeDescriptor edgeDescriptor(
        const NodeDescriptor& tailNode, const Edge& e) const;
    EdgeDescriptor edgeDescriptor(
        const Edge& e,const NodeDescriptor& headNode) const;
    NodeDescriptor descriptor(const Node& n) const;
    bool hasEdge(
        const Node& nTail,
        const Node& nHead,
        const Edge& edge) const;
    bool hasEdge(
        const Edge& edge) const;
    
    EdgeDescriptor connectingEdge(
        const Node& nTail,
        const Node& nHead) const;
    
    EdgeSet connectingEdges(
        const Node& nTail, const Node& nHead) const;
    
    // optimized but uglier versions
    Node& tailNode(const Edge& edge, const NodeDescriptor& headNode) const
        throw (InstanceNotFound);
    Node& headNode(const Edge& edge, const NodeDescriptor& tailNode) const
        throw (InstanceNotFound);
    
    // internal implementation of path-length-related things.
    void calculatePathLengths() const;
    void calculateSourceDistance(const GraphNode& node, int len) const;
    void calculateSinkDistance(const GraphNode& node, int len) const;
    
    virtual int edgeWeight( GraphEdge& e, const GraphNode& n) const;
    
    // Calculated path lengths
    mutable std::map<const GraphNode*,int, typename GraphNode::Comparator>
    sourceDistances_;
    mutable std::map<const GraphNode*,int, typename GraphNode::Comparator> 
    sinkDistances_;
    
    mutable int height_;

    /// The internal graph structure.
    Graph graph_;

    // these cache data that may get cached even on ro operations,
    // so they are mutable.
    typedef hash_map<const Edge*,EdgeDescriptor,GraphHashFunctions >
    EdgeDescMap;
    typedef hash_map<const Node*,NodeDescriptor,GraphHashFunctions >
    NodeDescMap;

    mutable EdgeDescMap edgeDescriptors_;
    mutable NodeDescMap nodeDescriptors_;

    // for subgraphs
    BoostGraph<GraphNode, GraphEdge>* parentGraph_;
    std::vector<BoostGraph<GraphNode,GraphEdge>*> childGraphs_;

    // graph editing functions which tell the changes to parents and childs

    virtual void removeNode(Node& node, BoostGraph* modifierGraph)
        throw (InstanceNotFound);
    virtual void removeEdge(Edge& e, BoostGraph* modifierGraph)
        throw (InstanceNotFound);    

    virtual void connectNodes(
        const Node& nTail, const Node& nHead, Edge& e,
        GraphBase<GraphNode, GraphEdge>* modifier)
        throw (ObjectAlreadyExists);

    void moveInEdges(
        const Node& source, const Node& destination,BoostGraph* modifierGraph)
        throw (NotAvailable);
    virtual void moveOutEdges(
        const Node& source, const Node& destination,BoostGraph* modifierGraph)
        throw (NotAvailable);
    void constructSubGraph(
        BoostGraph& subGraph, NodeSet& nodes);

    std::string name_;
    int sgCounter_;
};

#include "BoostGraph.icc"

#endif
