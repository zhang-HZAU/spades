//***************************************************************************
//* Copyright (c) 2015 Saint Petersburg State University
//* Copyright (c) 2011-2014 Saint Petersburg Academic University
//* All Rights Reserved
//* See file LICENSE for details.
//***************************************************************************

#pragma once

#include "assembly_graph/stats/picture_dump.hpp"
#include <io/reads/osequencestream.hpp>
#include "assembly_graph/components/connected_component.hpp"
#include "assembly_graph/stats/statistics.hpp"
#include "assembly_graph/paths/path_finders.hpp"
#include "assembly_graph/paths/path_utils.hpp"
#include "assembly_graph/paths/bidirectional_path_io/io_support.hpp"

namespace debruijn_graph {

inline void OutputEdgeSequences(const Graph &g, const std::string &contigs_output_filename) {
    INFO("Outputting contigs to " << contigs_output_filename << ".fasta");
    io::osequencestream_cov oss(contigs_output_filename + ".fasta");

    for (EdgeId e: g.canonical_edges()) {
        oss << g.coverage(e);
        oss << g.EdgeNucls(e).str();
    }
}

inline void OutputEdgesByID(const Graph &g,
                                const std::string &contigs_output_filename) {
    INFO("Outputting contigs to " << contigs_output_filename << ".fasta");
    io::OFastaReadStream oss(contigs_output_filename + ".fasta");
    for (EdgeId e: g.canonical_edges()) {
        std::string s = g.EdgeNucls(e).str();
        oss << io::SingleRead(io::MakeContigId(g.int_id(e), s.size(), g.coverage(e), "EDGE"), s);
    }
}
} // namespace debruijn_graph

