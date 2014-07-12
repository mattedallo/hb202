int transponders_utilization_compare(PrgT_Vector *path1_vptr, PrgT_Vector *path2_vptr) {
    /*1 if elem1 should be closer to the vector head than elem2
    0 if elem1 and elem2 are equal in rank
    -1 if elem2 should be closer to the vector head than elem1 */
    FIN( transponders_utilization_compare(path1_vptr,path2_vptr) );
    edge1_ptr = prg_vector_access (path1_vptr, 0);
    edge2_ptr = prg_vector_access (path1_vptr, 0);
    // get the transponder vertex
    transponder_vertex1_ptr = prg_edge_vertex_b_get (edge1_ptr);
    transponder_vertex2_ptr = prg_edge_vertex_b_get (edge2_ptr);
    
    transponder_1_ptr = topology_fake_transponder_vertex_transponder_get(gbl_topology_ptr,transponder_vertex1_ptr);
    transponder_2_ptr = topology_fake_transponder_vertex_transponder_get(gbl_topology_ptr,transponder_vertex2_ptr);
    utilization1 = transponder_utilization(transponder_1_ptr);
    utilization2 = transponder_utilization(transponder_2_ptr);
    if (utilization1>utilization2) {
        FRET(1);
    }
    else if (utilization1<utilization2)
        FRET(-1);
    }
    else {
        FRET(0);
    }
}



    PrgT_Bin_Hash_Table     *transponders_hptr;
    PrgT_List_Cell 			*cell_ptr;
    PrgT_List               *transponders_lptr;
    int                     num_carriers,carrier_spacing;
    PrgT_Graph_Vertex       *fake_source_vertex_ptr,transponder_vertex_ptr,
    Transponder             *transponder_ptr;

    // get the number of slices to allocate
    cb_closer_allowed_capacity_slices_get(&capacity_under_allocation, &required_slices);
    // get the number of carriers required
    num_carriers = cb_capacity_to_num_carriers(capacity_under_allocation);
    //for simplicity assume the spacing between carriers as follow
    if (num_carriers > 1) {
        carrier_spacing = (required_slices*2)/num_carriers; //in terms of frequency indexes
    }
    else {
        carrier_spacing = 0;
    }
    // create the fake source vertex
    fake_source_vertex_ptr = topology_new_fake_vertex(topology);

    // create a list of fake transponder vertices used to deallocate them later
    fake_transponders_vertices_lptr = prg_list_create();

    // get the list of transponder
    transponders_hptr = ted_node_transponders_get(topology, source_addr_ptr);
    transponders_lptr = prg_bin_hash_table_item_list_get(transponders_hptr);
    //foreach transponder check if 
    //  1. it has enough free carriers
    //  2. has a spacing constraints according to the required spacing  
    cell_ptr = prg_list_head_cell_get(transponders_lptr);
    while (PRGC_NIL != cell_ptr) {
        transponder_ptr = (Transponder *)prg_list_cell_data_get (cell_ptr);
        if (num_carriers <= transponder_num_free_carriers(transponder_ptr) &&
            carrier_spacing <= transponder_maximum_spacing_get(transponder_ptr) &&
            carrier_spacing >= transponder_minimum_spacing_get(transponder_ptr)
            ) {
            if (transponder_has_fixed_spacing(transponder_ptr)) {
                fixed_spacing = transponder_fixed_spacing_get(transponder_ptr);
                if (carrier_spacing % fixed_spacing != 0 )
                    continue;//the carrier spacing is not a multiple of the fixed spacing
            }
            // compute the spectrum map for the transponder
            spectrum_map_ptr = transponder_spectrum_map_by_demand_get(transponder_ptr,num_carriers,carrier_spacing);
            // in case the spectrum map is empty, skip the transponder
            if (spectrum_map_ptr == PRGC_NIL)
                continue;
            // create the new vertex
            transponder_vertex_ptr = topology_new_fake_transponder_vertex(transponder_ptr);
            // insert the transponder vertex into the list
            prg_list_insert (fake_transponders_vertices_lptr, transponder_vertex_ptr, PRGC_LISTPOS_HEAD);
            // attach the new vertex to the fake source and to the real source
            edge_fsource_to_trans_ptr = prg_graph_edge_insert (topology->graph_ptr, fake_source_vertex_ptr, transponder_vertex_ptr, PrgC_Graph_Edge_Simplex);
            edge_trans_to_rsource_ptr = prg_graph_edge_insert (topology->graph_ptr, transponder_vertex_ptr, source_vertex_ptr, PrgC_Graph_Edge_Simplex);
            // set the edge spectrum between fake source vertex and 
            // transponder vertex as totally free
            te_edge_spectrum_map_set(topology,edge_fsource_to_trans_ptr, spectrum_map_ptr);
            // set the edge spectrum between transponder vertex 
            // and real source vertex according to the state of the transponder
            te_edge_spectrum_map_set(topology,edge_trans_to_rsource_ptr, spectrum_map_ptr);
        }
        cell_ptr = prg_list_cell_next_get(cell_ptr);
    }
    // destroy the temporary list of transponder (not the transponder itself)
    prg_list_destroy (transponders_lptr, PRGC_FALSE);

    // compute the shortest+k paths
    min_num_hops = pce_find_minimum_hops_number(topology, source_vertex_ptr, dest_vertex_ptr)+4; //4 are the extra hops given by the fake nodes
    computed_paths_vptr = prg_djk_k_paths_compute_hop_limit(fake_source_vertex_ptr, fake_dest_vertex_ptr, path_pool_limit, min_num_hops + additional_hops);

    // order the paths based on transponder utilization (from the most used to the least)
    current_gbl_topology = topology; // used by the transponder_utilization_compare function
    prg_vector_sort (computed_paths_vptr, transponders_utilization_compare);


    // loop the sorted paths vector
    size = prg_vector_size(computed_paths_vptr);
    for (i=0; i<size; ++i) {
        edges_vptr = prg_vector_access(computed_paths_vptr,i);
        // compute the spectrum sum of all the edges
        path_spectrum_map_ptr = te_edges_spectrum_sum(topology,edges_vptr);
        // in case the spectrum is not null get a free central frequency
        // according to the allocation policy
        if (PRGC_NIL != path_spectrum_map_ptr ) {
             if (spectrum_map_first_free_frequency_get(path_spectrum_map_ptr,&central_frequency_index) == MD_Compcode_Failure) {
                spectrum_map_destroy( path_spectrum_map_ptr);
                continue;
             }
             // add the selected central_frequency_index to the path comp reply
             
             // get the suggested transponders from the path
             
             // remove the fake nodes from the path
             
             // compute the carriers associated to the central frequency
             
             // add everything tho the path comp reply
             
             break;
        }
    }

    // destroy the fake vertices and edges
    topology_vertex_remove_and_destroy(topology, fake_source_vertex_ptr);
    cell_ptr = prg_list_head_cell_get(list_ptr);
    while (PRGC_NIL != cell_ptr) {
         transponder_vertex_ptr = prg_list_cell_data_get(cell_ptr);
         topology_vertex_remove_and_destroy(topology, transponder_vertex_ptr);
         cell_ptr = prg_list_cell_next_get(cell_ptr);
    }
    prg_list_destroy (fake_transponders_vertices_lptr, PRGC_FALSE);

    // return the path comp reply
    // it should contain: computed path, the suggested frequency, carriers and transponders


    // IMPROVEMENTS:
    // the paths can be computed just ones
    // and we can avoid to create and destroy the fake vertices every time


