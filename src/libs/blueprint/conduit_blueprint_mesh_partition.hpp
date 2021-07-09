// Copyright (c) Lawrence Livermore National Security, LLC and other Conduit
// Project developers. See top-level LICENSE AND COPYRIGHT files for dates and
// other details. No copyright assignment is required to contribute to Conduit.

//-----------------------------------------------------------------------------
///
/// file: conduit_blueprint_mesh_partition.hpp
///
//-----------------------------------------------------------------------------

#ifndef CONDUIT_BLUEPRINT_MESH_PARTITION_HPP
#define CONDUIT_BLUEPRINT_MESH_PARTITION_HPP

//-----------------------------------------------------------------------------
// std includes
//-----------------------------------------------------------------------------
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

//-----------------------------------------------------------------------------
// conduit lib includes
//-----------------------------------------------------------------------------
#include "conduit.hpp"
#include "conduit_blueprint_exports.h"

//-----------------------------------------------------------------------------
// -- begin conduit --
//-----------------------------------------------------------------------------
namespace conduit
{

//-----------------------------------------------------------------------------
// -- begin conduit::blueprint --
//-----------------------------------------------------------------------------
namespace blueprint
{

//-----------------------------------------------------------------------------
// -- begin conduit::blueprint::mesh --
//-----------------------------------------------------------------------------
namespace mesh
{

/**
 @brief Base class for selections that identify regions of interest that will
        be extracted from a mesh.
 */
class selection
{
public:
    selection();
    virtual ~selection();

    /**
     @brief Initializes the selection from the provided Conduit node.
     @param n_options A Conduit node containing the options for this
                      selection.
     @return True if the selection was initialized successfully; False otherwise.
     */
    virtual bool init(const conduit::Node &n_options);

    /**
     @brief Determines whether the selection can be applied to the supplied mesh.
     @note  This is a check that the selection type makes sense for the input
            mesh type.
     @param n_mesh A Conduit conduit::Node containing the mesh.
     @return True if the selection can be applied to the mesh; False otherwise.
     */
    virtual bool applicable(const conduit::Node &n_mesh) = 0;

    /**
     @brief Return the number of cells in the selection.
     @return The number of cells in the selection.
     */
    virtual index_t length() const;

    /**
     @brief Returns whether the selection covers the whole mesh. Selections that
            covered a subset of the mesh or were otherwise partitioned would
            not be whole.
     @param n_mesh A Conduit conduit::Node containing the mesh.
     @return True of the mesh is whole. False otherwise.
     */
    virtual bool get_whole(const conduit::Node &n_mesh);

    /**
     @brief Set whether the selection is considered to cover the whole mesh.
     @param value A value indicating whether the selection covers the mesh.
     */
    void set_whole(bool value);

    /**
     @brief Partitions the selection into smaller selections.
     @param n_mesh A Conduit conduit::Node containing the mesh.
     @return A vector of selection pointers that cover the input selection.
     */
    virtual std::vector<std::shared_ptr<selection> > partition(const conduit::Node &n_mesh) const = 0;

    /**
     @brief Return the domain index to which the selection is being applied.
     @return The domain index or 0. This value is 0 by default.
     */
    index_t get_domain() const;

    /**
     @brief Set the domain index to which the selection is being applied.
     @param value The new domain index.
     */
    void set_domain(index_t value);

    /**
     @brief Get the name of the topology for this selection.
     */
    const std::string &get_topology() const;

    /**
     @brief Set the topology used for selection. The topology needs to be 
            valid for the selection mesh.
     @param value The new topology name.
     */
    void set_topology(const std::string &value);  

    /**
     @brief Returns the cells in this selection that are contained in the
            supplied topology. Such cells will have cell ranges in erange,
            inclusive. The element ids are returned in element_ids.
     */
    virtual void get_element_ids_for_topo(const conduit::Node &n_topo,
                                          const index_t erange[2],
                                          std::vector<index_t> &element_ids) const = 0;

    /**
     @brief Returns a topology node that we can use from the mesh.
     @note This method throws an exception if the topology does not exist.
     @param n_mesh A Conduit node that contains the mesh.
     @return A reference to the selected topology node.
     */
    const conduit::Node &selected_topology(const conduit::Node &n_mesh) const;

    /**
     @brief Prints the selection to the stream.
     @param os The stream to which the information will be printed.
     */
    virtual void print(std::ostream &os) const = 0;

protected:
    /**
     @brief Determines whether the selection covers the whole mesh.
     @param n_mesh A Conduit conduit::Node containing the mesh.
     @return True of the mesh is whole. False otherwise.
     */
    virtual bool determine_is_whole(const conduit::Node &n_mesh) const = 0;

    static const std::string DOMAIN_KEY;
    static const std::string TOPOLOGY_KEY;

    enum {
        WHOLE_UNDETERMINED,
        WHOLE_DETERMINED_FALSE,
        WHOLE_DETERMINED_TRUE
    };

    int         whole;
    index_t     domain;
    std::string topology;
};

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
/**
 @brief This class can read a set of selections and apply them to a Conduit
        node containing single or multi-domain meshes and produce a new
        Conduit node that refashions the selections into a target number of
        mesh domains. This is the serial implementation.
 */
class partitioner
{
public:
    /**
     @brief This struct is a Conduit/Blueprint mesh plus an ownership bool.
            The mesh pointer is always assumed to be external by default
            so we do not provide a destructor. If we want to delete it,
            call free(), which will free the mesh if we own it.
     */
    struct chunk
    {
        chunk();
        chunk(const Node *m, bool own);
        void free();

        const Node *mesh;
        bool        owns;
    };

    /**
     @brief Constructor.
     */
    partitioner();

    /**
     @brief Destructor.
     */
    virtual ~partitioner();

    /**
     @brief Initialize the partitioner using the input mesh (which could be
            multidomain) and a set of options. The options specify how we
            may be pulling apart the mesh using selections. The selections
            are allowed to be empty, in which case we're using all of the
            input mesh domains.

     @note We initialize before execute() because we may want the opportunity
           to split selections into an appropriate target number of domains.

     @param n_mesh A Conduit node representing the Blueprint mesh.
     @param options A Conduit node containing the partitioning options.

     @return True if the options were accepted or False if they contained
             an error.
     */
    bool initialize(const conduit::Node &n_mesh, const conduit::Node &options);

    /**
     @brief This method enables the partitioner to split the selections until
            we arrive at a number of selections that will yield the desired
            number of target domains.

     @note We could oversplit at some point if we're already unstructured
           if we want to load balance mesh sizes a little better.
     */
    virtual void split_selections();

    /**
     @brief Execute the partitioner so we use any options to arrive at the
            target number of domains. This may involve splitting domains,
            redistributing them (in parallel), and then combining them to
            populate the output node.

     @param[out] output The Conduit node that will receive the output meshes.
     */
    void execute(conduit::Node &output);


    /**
     @brief Given a vector of input Blueprint meshes, we combine them into 
            a single Blueprint mesh stored in output.

     @param domain The domain number being created out of the various inputs.
     @param inputs A vector of Blueprint meshes representing the input
                   meshes that will be combined.
     @param[out] output The Conduit node to which the combined mesh's properties
                        will be added.

     @note This method is exposed so we can create a partitioner object and 
           combine meshes with it directly, which would be useful for development
           and unit tests. This method is serial only and will only operate on
           its inputs to generate the single combined mesh for the output node.
     */
    void combine(int domain,
                 const std::vector<const Node *> &inputs,
                 Node &output);

protected:
    /**
     @brief Compute the total number of selections across all ranks.
     @return The total number of selections across all ranks.

     @note Reimplement in parallel
     */
    virtual long get_total_selections() const;

    /**
     @brief Get the rank and index of the largest selection. In parallel, we
            will be looking across all ranks to find the largest domains so
            those are split first.

     @note  Splitting one at a time is temporary since in parallel, it's not
            good enough. One of the reasons we split one at a time right now
            is to allow really large selections to be split more than once 
            before other selections are considered.

     @param[out] sel_rank The rank that contains the largest selection.
     @param[out] sel_index The index of the largest selection on sel_rank.

     @note Reimplement in parallel
     */
    virtual void get_largest_selection(int &sel_rank, int &sel_index) const;

    /**
     @brief This is a factory method for creating selections based on the 
            provided options.
     @param type The name of the selection type to create.
     @return A new instance of the requested selection type.
     */
    virtual std::shared_ptr<selection> create_selection(const std::string &type) const;

    /**
     @brief Create a selection of the type that best selects all of the 
            elements in the supplied mesh. For example, if we're given a 
            structured mesh, we'd return a logical selection that spans
            all cells in the mesh.
     @param n_mesh A Conduit node that contains the mesh we're selecting.
     @return A new selection object that selects all cells in the mesh.
     */
    std::shared_ptr<selection> create_selection_all_elements(const conduit::Node &n_mesh) const;

    void copy_fields(const std::vector<index_t> &all_selected_vertex_ids,
                     const std::vector<index_t> &all_selected_element_ids,
                     const conduit::Node &n_mesh,
                     conduit::Node &output,
                     bool preserve_mapping) const;

    void copy_field(const conduit::Node &n_field,
                    const std::vector<index_t> &ids,
                    Node &n_output_fields) const;

    void slice_array(const conduit::Node &n_src_values,
                     const std::vector<index_t> &ids,
                     Node &n_dest_values) const;

    void get_vertex_ids_for_element_ids(const conduit::Node &n_topo,
             const std::vector<index_t> &element_ids,
             std::set<index_t> &vertex_ids) const;

    /**
     @brief Extract the idx'th selection from the input mesh and return a
            new Node containing the extracted chunk.

     @param idx The selection to extract. This must be a valid selection index.
     @param n_mesh A Conduit node representing the mesh to which we're
                   applying the selection.
     @return A new Conduit node (to be freed by caller) that contains the
             extracted chunk.
     */
    conduit::Node *extract(size_t idx, const conduit::Node &n_mesh) const;

    void create_new_explicit_coordset(const conduit::Node &n_coordset,
             const std::vector<index_t> &vertex_ids,
             conduit::Node &n_new_coordset) const;

    /**
     @brief Creates a new unstructured topology from a subset of the 
            input topology. Any topologies that are not unstructured are
            currently converted to unstructured.

     @param n_topo A Conduit node containing source topology.
     @param csname The name of the coordset to use in the new topology.
     @param element_ids The element ids to include from the source topology.
     @param vertex_ids The vertex ids that are used from the source topology's coordset.
     @param n_new_topo A Conduit node that will contain the new topology.
     */
    void create_new_unstructured_topo(const conduit::Node &n_topo,
             const std::string &csname,
             const std::vector<index_t> &element_ids,
             const std::vector<index_t> &vertex_ids,
             conduit::Node &n_new_topo) const;

    /**
     @brief Creates a new unstructured topology from a subset of the 
            input unstructured topology.

     @param n_topo A Conduit node containing source topology.
     @param csname The name of the coordset to use in the new topology.
     @param element_ids The element ids to include from the source topology.
     @param vertex_ids The vertex ids that are used from the source topology's coordset.
     @param n_new_topo A Conduit node that will contain the new topology.
     */
    void unstructured_topo_from_unstructured(const conduit::Node &n_topo,
             const std::string &csname,
             const std::vector<index_t> &element_ids,
             const std::vector<index_t> &vertex_ids,
             conduit::Node &n_new_topo) const;

    /**
     @brief Given a set of input meshes which may have various topologies,
            recommend a topology that can be used to capture the combined
            meshes in a single output.

     @note If the meshes contain multiple topologies then it probably
           makes sense to recommend unstructured.

     @param inputs A vector of Blueprint mesh node pointers to consider.

     @return The recommended topology type that can represent the input
             meshes as a single mesh once they are combined.
     */
    std::string recommended_topology(const std::vector<const Node *> &inputs) const;

    /**
     @brief Given a set of inputs that are predetermined to fit together
            into a logically structured output, perform the actual
            recombination to yield a single mesh (uniform, rectilinear, ...)
            in the output.

     @param domain The input domain number (in case we want to record it)
     @param inputs A vector of Blueprint mesh nodes to combine.
     @param output The Conduit node into which the combined mesh output
                   will be added.
     */
    void combine_as_structured(int domain,
                               const std::vector<const Node *> &inputs,
                               Node &output);

    /**
     @brief Given a set of inputs that are of various types, assemble them
            into a single output mesh with unstructured topology. This
            method combines like-named coordsets and topologies.

     @param domain The input domain number (in case we want to record it)
     @param inputs A vector of Blueprint mesh nodes to combine.
     @param output The Conduit node into which the combined mesh output
                   will be added.
     */
    void combine_as_unstructured(int domain,
                                 const std::vector<const Node *> &inputs,
                                 Node &output);

    /**
     @brief Given a local set of chunks, figure out starting domain index
            that will be used when numbering domains on a rank. We figure
            out which ranks get domains and this is the scan of the domain
            numbers.
     
     @return The starting domain index on this rank.
     */
    virtual unsigned int starting_index(const std::vector<chunk> &chunks);

    /**
     @brief Assign the chunks on this rank a destination rank to which it will
            be transported as well as a destination domain that indices which
            chunks will be combined into the final domains.

     @note All chunks that get the same dest_domain must also get the same
           dest_rank since dest_rank is the MPI rank that will do the work
           of combining the chunks it gets. Also, this method nominates
           ranks as those who will receive chunks. If there are 4 target
           chunks then when we run in parallel, 4 ranks will get a domain.
           This will be overridden for parallel.

     @param chunks A vector of input chunks that we are mapping to ranks
                   and domains.
     @param[out] dest_ranks The destination ranks that get each input chunk.
     @param[out] dest_domain The destination domain to which a chunk is assigned.
     */
    virtual void map_chunks(const std::vector<chunk> &chunks,
                            std::vector<int> &dest_ranks,
                            std::vector<int> &dest_domain);

    /**
     @brief Communicates the input chunks to their respective destination ranks
            and passes out the set of chunks that this rank will operate on in
            the chunks_to_assemble vector.

     @param chunks The vector of input chunks that may be redistributed to
                   different ranks.
     @param dest_rank A vector of integers containing the destination ranks of
                      each chunk.
     @param dest_domain The global numbering of each input chunk.
     @param[out] chunks_to_assemble The vector of chunks that this rank will
                                    combine and set into the output.
     @param[out] chunks_to_assemble_domains The global domain numbering of each
                                            chunk in chunks_to_assemble. Like-
                                            numbered chunks will be combined
                                            into a single output domain.

     @note This will be overridden for parallel.
     */
    virtual void communicate_chunks(const std::vector<chunk> &chunks,
                                    const std::vector<int> &dest_rank,
                                    const std::vector<int> &dest_domain,
                                    std::vector<chunk> &chunks_to_assemble,
                                    std::vector<int> &chunks_to_assemble_domains);

    int rank, size;
    unsigned int target;
    std::vector<const Node *>                meshes;
    std::vector<std::shared_ptr<selection> > selections;
    std::vector<std::string>                 selected_fields;
};

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
/**
 @brief Partition an input mesh or set of mesh domains into a different decomposition,
        according to options. This is the serial implementation.
 @param n_mesh  A Conduit node containing a Blueprint mesh or set of mesh domains.
 @param options A Conduit node containing options that govern the partitioning.
 @param[out]    A Conduit node to accept the repartitioned mesh(es).
 */
void CONDUIT_BLUEPRINT_API partition(const conduit::Node &n_mesh,
                                     const conduit::Node &options,
                                     conduit::Node &output);

}
//-----------------------------------------------------------------------------
// -- end conduit::blueprint::mesh --
//-----------------------------------------------------------------------------

}
//-----------------------------------------------------------------------------
// -- end conduit::blueprint --
//-----------------------------------------------------------------------------

}
//-----------------------------------------------------------------------------
// -- end conduit:: --
//-----------------------------------------------------------------------------


#endif