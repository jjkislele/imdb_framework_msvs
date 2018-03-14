/*
Copyright (C) 2012 Mathias Eitz and Ronald Richter.
All rights reserved.

This file is part of the imdb library and is made available under
the terms of the BSD license (see the LICENSE file).
*/

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <direct.h>
#include <io.h>

#include <boost/algorithm/string.hpp>

#include <opencv2/highgui/highgui.hpp>

//util/
#include <types.hpp>
#include <quantizer.hpp>
#include <kmeans.hpp>
//io/
#include <property_reader.hpp>
#include <cmdline.hpp>
#include <filelist.hpp>
//descriptors/
#include <generator.hpp>
//search/
#include <linear_search.hpp>
#include <bof_search_manager.hpp>
#include <linear_search_manager.hpp>
#include <distance.hpp>
//myIO/
//#include "myIO.h"


//#define SHOW_RESULT
#define SAVE_SCORE

using namespace imdb;

template <class search_t>
void image_search(const anymap_t& data, const search_t& search, size_t num_results, vector<dist_idx_t>& results)
{
    typedef typename search_t::descr_t descr_t;
    descr_t descr = get<descr_t>(data, "features");
    search.query(descr, num_results, results);
}



class command_search : public Command
{
public:

    command_search()
        : Command("image_search [options]")
        , _co_query_image("queryimage"        , "q", "filename of images filelist to be used as the query [required]")
        , _co_search_ptree("searchptree"      , "s", "filename of the JSON file containing parameters for the search manager [optional, if not provided, --searchparams must be given]")
        , _co_search_params("searchparams"    , "m", "parameters for the search manager [optional, if not provided, --searchptree must be given]")
        , _co_vocabulary("vocabulary"         , "v", "filename of vocabulary used for quantization [optional, only required with bag-of-features search]")
        , _co_filelist("filelist"             , "l", "filename of images filelist [required]")
        , _co_generator_name("generatorname"  , "g", "name of generator [optional, if given, we will use generator's default parameters and ignore --generatorptree]")
        , _co_generator_ptree("generatorptree", "p", "filename of the JSON file containing generator name and parameters [optional, if not provided, generator's default values are used']")
        , _co_num_results  ("numresults"      , "n", "number of results to search for [optional, if not provided all distances get computed]")
		, _co_wkdir("working dir"             , "r", "directory path of the query [required]")
		, _co_outdir("saving dir"             , "o", "the path of the retrieval list saved in [optional, if not provided, will be set as \"retrieval_list\"]")
    {
        add(_co_query_image);
        add(_co_search_ptree);
        add(_co_search_params);
        add(_co_vocabulary);
        add(_co_filelist);
        add(_co_generator_ptree);
        add(_co_num_results);
        add(_co_generator_name);
		add(_co_wkdir);
		add(_co_outdir);
    }


    bool run(const std::vector<std::string>& args)
    {
        if (args.size() == 0)
        {
            print();
            return false;
        }

        warn_for_unknown_option(args);


        string in_queryimage;
        string in_searchptree;
        string in_filelist;
        string in_generatorptree;
        string in_generatorname;
        string in_vocabulary;
		string in_wkdir;
		string in_outdir;

        // this default value will make the search managers search
        // for all images if the user does not provide a value
        size_t in_numresults = std::numeric_limits<size_t>::max();


        // check that the required options are available
        if (!_co_query_image.parse_single<string>(args, in_queryimage) || !_co_filelist.parse_single<string>(args, in_filelist) || !_co_wkdir.parse_single(args, in_wkdir))
        {
            print();
            return false;
        }


        // -----------------------------------------------------------------------------------
        // either searchptree or searchparams must be given
        ptree search_params;
        vector<string> in_searchparams;
        if (_co_search_ptree.parse_single<string>(args, in_searchptree))
        {
            boost::property_tree::read_json(in_searchptree, search_params);
        }
        else if (_co_search_params.parse_multiple<string>(args, in_searchparams))
        {
            for (size_t i = 0; i < in_searchparams.size(); i++)
            {
                vector<string> pv;
                boost::algorithm::split(pv, in_searchparams[i], boost::algorithm::is_any_of("="));

                if (pv.size() != 1 && pv.size() != 2)
                {
                    std::cerr << "image_search: cannot parse search manager parameter: " << in_searchparams[i] << std::endl;
                    return false;
                }
                search_params.put(pv[0], (pv.size() == 2) ? pv[1] : "");
            }

        }
        else
        {
            print();
            return false;
        }
        // -----------------------------------------------------------------------------------



        // try to parse the optional num_results parameter
        _co_num_results.parse_single<size_t>(args, in_numresults);
		// try to parse the optional outdir parameter
		if (!_co_outdir.parse_single<string>(args, in_outdir)) {
			in_outdir = "retrieval_list";
		}

        // -----------------------------------------------------------------
        // create the generator; we have the following rule:
        // a) if the user provides a generator name, we use this and ignore an additional generator ptree
        // b) if no generator name but ptree is provided, we use this
        shared_ptr<Generator> gen;
        if (_co_generator_name.parse_single<string>(args, in_generatorname))
        {
            gen = Generator::from_default_parameters(in_generatorname);
        }
        else if (_co_generator_ptree.parse_single<string>(args, in_generatorptree))
        {
            gen = Generator::from_parameters_file(in_generatorptree);
        }
        else
        {
            std::cerr << "image_search: must provide either generator name or ptree" << std::endl;
            std::cerr << "received args: generatorname=" << in_generatorname << "; generatorptree=" << in_generatorptree << std::endl;

            print();
            return false;
        }
        // -----------------------------------------------------------------

		// load img set file list
        FileList imageFiles;
        imageFiles.load(in_filelist);
		// load query img set file list
		FileList queryFiles;

		queryFiles.set_root_dir(in_wkdir);
		queryFiles.load(in_queryimage);


		// -----------------------------------------------------------------
		// processing
		// -----------------------------------------------------------------
		std::cout << "----------------------------" << std::endl;
		std::cout << "image_search: now processing..." << std::endl;
		std::cout << "image_search: working dir is " << in_wkdir << std::endl;
		for (int i = 0; i < queryFiles.size(); i++)
		{
			std::cout << "image_search: progress " << i + 1 << '/' << queryFiles.size() << std::endl;

			std::istringstream sin(queryFiles.get_relative_filename(i));
			string retrievalClass;
			string retrievalFile;
			std::getline(sin, retrievalClass, '/');
			std::getline(sin, retrievalFile, '.');
			string outRetrievalListDir = in_outdir + '/' + retrievalClass;
			string outRetrievalListPath = in_outdir + '/' + retrievalClass + '/' + retrievalFile;
			if ((_access(outRetrievalListPath.c_str(), 0)) != -1) {
				std::cout << "image_search: progress " << i + 1 << " skipped" << std::endl;
				continue;
			}
			
			string each_queryimg = queryFiles.get_filename(i);
			mat_8uc3_t image = cv::imread(each_queryimg, 1);
			anymap_t data;
			data["image"] = image;

			gen->compute(data);

			vector<dist_idx_t> results;
			
			if (search_params.get<std::string>("search_type") == "BofSearch")
			{

				// parameter --vocabulary must be given
				if (!_co_vocabulary.parse_single<string>(args, in_vocabulary))
				{
					std::cerr << "image_search: when using bag-of-features search, you must also provide the --vocabulary commandline option" << std::endl;
					print();
					return false;
				}


				vec_vec_f32_t vocabulary;
				read_property(vocabulary, in_vocabulary);

				// quantize
				quantize_fn quantizer = quantize_hard<vec_f32_t, imdb::l2norm_squared<vec_f32_t> >();
				vec_vec_f32_t quantized_samples;

				const vec_vec_f32_t& samples = boost::any_cast<vec_vec_f32_t>(data["features"]);
				quantize_samples_parallel(samples, vocabulary, quantized_samples, quantizer);

				vec_f32_t histvw;
				build_histvw(quantized_samples, vocabulary.size(), histvw, false);

				// initialize search manager and run query
				BofSearchManager bofSearch(search_params);
				bofSearch.query(histvw, in_numresults, results);
			}

			else if (search_params.get<std::string>("search_type") == "LinearSearch")
			{
				// Tensor descriptor is a bit of a special case as we additionally
				// need to pass a 'mask' to the distance function, so we replicate
				// most of the functionality implemented in LinearSearchManager
				if (gen->parameters().get<string>("name") == "tensor")
				{
					string filename = search_params.get<string>("descriptor_file");
					vec_vec_f32_t features;
					read_property(features, filename);

					const vec_f32_t& descr = get<vec_f32_t>(data, "features");
					const vector<bool>& mask = get<vector<bool> >(data, "mask");
					std::cout << "mask size=" << mask.size() << std::endl;
					dist_frobenius<vec_f32_t> distfn;
					distfn.mask = &mask;
					linear_search(descr, features, results, in_numresults, distfn);

				}
				else
				{
					LinearSearchManager search(search_params);
					image_search(data, search, in_numresults, results);
				}
			}
			else
			{
				std::cerr << "unsupported search type" << std::endl;
			}


#ifdef SHOW_RESULT
			// output results on the console
			// use piping to store in a text file (for now)
			std::cout << "----------------------------" << std::endl;
			std::cout << "-idx--score--filename-------" << std::endl;
			std::cout << "----------------------------" << std::endl;
			for (size_t i = 0; i < results.size(); i++) {
				string filename = imageFiles.get_relative_filename(results[i].second);
				std::cout.precision(9);
				std::cout << i << " " << (float)results[i].first << " " << filename << std::endl;
			}
#endif // SHOW_RESULT

			// store as csv
			// mkdir outdir/class
			if (_mkdir(in_outdir.c_str()) != -1) {
				printf("\nCreate Dir: %s\n", in_outdir.c_str());
			}
			if (_mkdir(outRetrievalListDir.c_str()) != -1) {
				printf("\nCreate Dir: %s\n", outRetrievalListDir.c_str());
			}

			std::ofstream str;
			str.open(outRetrievalListPath.c_str());
			if (str.is_open()) {
				for (size_t i = 0; i < results.size(); i++) {
					string filename = imageFiles.get_relative_filename(results[i].second);
					std::istringstream sin1(filename);
					std::getline(sin1, filename, '.');
#ifdef SAVE_SCORE
					double score = results[i].first;
					str << std::to_string(score) << ' ' << filename << '\n';
#else
					str << filename << '\n';
#endif
				}
			}
		}



        return true;
    }

private:

    CmdOption _co_query_image;
    CmdOption _co_search_ptree;
    CmdOption _co_search_params;
    CmdOption _co_vocabulary;
    CmdOption _co_filelist;
    CmdOption _co_generator_name;
    CmdOption _co_generator_ptree;
    CmdOption _co_num_results;
	CmdOption _co_wkdir;
	CmdOption _co_outdir;
};



int main(int argc, char *argv[])
{

    command_search cmd;
    bool okay = cmd.run(argv_to_strings(argc-1, &argv[1]));
    return okay ? 0:1;
}
