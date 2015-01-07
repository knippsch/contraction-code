/*
 * GlobalData.cpp
 *
 *  Created on: Mar 28, 2013
 *      Author: knippsch
 */

#include "GlobalData.h"
#include "GlobalData_tests.hpp"

namespace po = boost::program_options;

GlobalData* GlobalData::instance_ = 0;

GlobalData* GlobalData::Instance () {

  if(instance_ == 0) instance_ = new GlobalData;

  return instance_;
}
// *****************************************************************************
/// @brief Convenience function for when a 'store_to' value is being provided
///        to typed_value.
///
/// @param store_to The variable that will hold the parsed value upon notify.
///
/// @return Pointer to a type_value.
template<typename T>
boost::program_options::typed_value<T>* make_value (T* store_to) {
  return boost::program_options::value<T>(store_to);
}
// *****************************************************************************
/// @brief Makes a quark object from a string
quark make_quark (const std::string& quark_string) {
  // Tokenize the string on the ":" delimiter.
  std::vector<std::string> tokens;
  boost::split(tokens, quark_string, boost::is_any_of(":"));

  // If the split did not result in exactly 8 tokens, then the value
  // is formatted wrong.
  if(8 != tokens.size()){
    using boost::program_options::validation_error;
    throw validation_error(validation_error::invalid_option_value,
        "quarks.quark", quark_string);
  }

  // Create a quark from the token values.
  return quark(tokens[0], boost::lexical_cast<int>(tokens[1]), tokens[2],
      boost::lexical_cast<int>(tokens[3]), tokens[4],
      boost::lexical_cast<int>(tokens[5]), tokens[6],
      boost::lexical_cast<int>(tokens[7]));
}

////////////////////////////////////////////////////////////////////////////////
//TODO: clean that up ////////////////////////////////////////////////////////// 
//init operator ////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GlobalData::init_from_infile() {

  //TODO: think about the numbers of momenta, displacements and gamma
  const size_t nb_mom = get_number_of_momenta(); //momentum_squared.size();
  const size_t nb_mom_sq = number_of_momentum_squared;
  //TODO: include displacement into dg (displacementgamma) multiindex
  const size_t nb_dg = number_of_displ_gamma;
  const size_t nb_dis = number_of_displ;

  // nb_op - number of combinations of three-momenta and gamma structures
  // op    - vector of all three-momenta, three-displacements and gamma 
  //         structure combinations
  const size_t nb_op = number_of_operators;
  op_Corr.resize(nb_op);
  const size_t nb_VdaggerV = nb_dis*(nb_mom/2+1);
  op_VdaggerV.resize(nb_VdaggerV);
  const size_t nb_rVdaggerVr = nb_dis*nb_mom;
  op_rVdaggerVr.resize(nb_rVdaggerVr);
  set_Corr();

  // nb_op_C2 - number of combinations of absolute values squared of momenta
  //            and gamma-displacement combinations for 2pt-fct
  // op_C2    - vector of all combinations for 2pt-fct and vector of 
  //            op-index-pairs with all corresponding three-vectors and gammas
  const size_t nb_op_C2 = nb_mom_sq * nb_dg * nb_dg;
  op_C2.resize(nb_op_C2);
  set_C2();

  const size_t nb_op_C4 = nb_mom_sq * nb_mom_sq * nb_dg * nb_dg;
  op_C4.resize(nb_op_C4);
  set_C4();

}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

// function to initialize the vector of all necessary operators
void GlobalData::set_Corr(){

  const size_t nb_mom = get_number_of_momenta(); //momentum_squared.size();
  const int max_mom_squared = number_of_max_mom;
  const size_t nb_dir = number_of_dirac;
  const size_t nb_dis = number_of_displ;
  const size_t nb_dg = dg.size();
  size_t nb_op = op_Corr.size();

  size_t i = 0;
  size_t j = 0;
  size_t k = 0;
  size_t p = 0;

  // all three-momenta
  for(int ipx = -max_mom_in_one_dir; ipx <= max_mom_in_one_dir; ++ipx){
    for(int ipy = -max_mom_in_one_dir; ipy <= max_mom_in_one_dir; ++ipy){
      for(int ipz = -max_mom_in_one_dir; ipz <= max_mom_in_one_dir; ++ipz){
        if((ipx * ipx + ipy * ipy + ipz * ipz) > max_mom_squared) {
          continue;
        }
        // all gamma structures
        for(size_t dis = 0; dis < nb_dis; dis++){
        for(size_t gam = 0; gam < nb_dir; gam++){

          op_Corr[i].p3[0] = ipx;
          op_Corr[i].p3[1] = ipy;
          op_Corr[i].p3[2] = ipz;
          op_Corr[i].dis3[0] = 0;
          op_Corr[i].dis3[1] = 0;
          op_Corr[i].dis3[2] = 0;
          op_Corr[i].gamma[0] = 5;
          op_Corr[i].gamma[1] = 4;
          op_Corr[i].gamma[2] = 4;
          op_Corr[i].gamma[3] = 4;
  
          if(p <= nb_mom/2)
            op_Corr[i].id_VdaggerV = j;
          else
            op_Corr[i].id_VdaggerV = nb_mom-nb_dis*(j/nb_dis+1)+j%nb_dis;

          op_Corr[i].id_rVdaggerVr = k;

          if(gam == 0){

            if(p <= nb_mom/2){
              op_VdaggerV[j].id = j;
              op_VdaggerV[j].index = i;

              op_Corr[i].flag_VdaggerV = 1;

              op_rVdaggerVr[k].adjoint = false;
            }
            else{
              op_Corr[i].flag_VdaggerV = -1;

              op_rVdaggerVr[k].adjoint = true;
              op_rVdaggerVr[k].id_adjoint = nb_mom-nb_dis*(k/nb_dis+1)+k%nb_dis;
            }

            op_rVdaggerVr[k].id = k;
            op_rVdaggerVr[k].index = i;

            j++;
            k++;
          }
          else{
            // NECESSARY that op_Corr blocks all indices with same VdaggerV
            op_Corr[i].flag_VdaggerV = 0;
          }

          if((p == nb_mom/2) && (dis == 0) && (gam == 0))
            index_of_unity = i;

          op_Corr[i].id = i;
          i++;
        }}
      p++;
      }
    }
  }

  if(i != op_Corr.size()){
    std::cout << "Error in LapH::set_Cor(): nb_op not equal to allocated "
                 "number of operators" << std::endl;
    exit(0);
  } 

}

// function to obtain the index combinations in op for the 2pt-fct
void GlobalData::set_C2(){

  const size_t nb_mom_sq = number_of_momentum_squared;
  const size_t nb_dg = dg.size();

  size_t nb_op = op_Corr.size();
  size_t j = 0;

  // run over all momenta squared (back-to-back hardcoded) and gamma 
  // combinations
  for(size_t p_sq = 0; p_sq < nb_mom_sq; p_sq++){
    for(size_t so = 0; so < nb_dg; so++){
      for(size_t si = 0; si < nb_dg; si++){

        // index for access of element
        size_t i = p_sq * nb_dg*nb_dg + so * nb_dg + si;

        // save p^2 and gamma structure at source and sink
//        op_C2[i].p_sq = p_sq;
//        op_C2[i].dg_so = so;
//        op_C2[i].dg_si = si;

        op_C2[i].id = j;

        // loop over op and set index pairs
        for(auto& el : op_Corr){
          if((el.p3[0]*el.p3[0] + el.p3[1]*el.p3[1] + el.p3[2]*el.p3[2]) == p_sq){ 
            if(el.gamma[0] == dg[so]){
              size_t id1 = el.id;
              // thats the generalized version of nb_mom - p - 1 including 
              // a faster running gamma structure
              size_t id2 = nb_op - nb_dg * (id1/nb_dg + 1) + si;
              // warning because array has no list-of-arrays constructor but 
              // works. Can change this to pair structure.
              op_C2[i].index.emplace_back(std::pair<size_t, size_t>(id1, id2));
            }
          }
        }

        j++;
        
      }
    }
  }

  if(j != op_C2.size()){
    std::cout << "Error in LapH::set_C2(): nb_op not equal to allocated "
                 "number of operators" << std::endl;
    exit(0);
  } 

}

// function to obtain the index combinations in op for the 2pt-fct
void GlobalData::set_C4(){

  const size_t nb_mom_sq = number_of_momentum_squared;
  const size_t nb_dg = dg.size();

  size_t nb_op = op_Corr.size();
  size_t i = 0;
  size_t j = 0;

  for(size_t so = 0; so < nb_dg; so++){
  for(size_t si = 0; si < nb_dg; si++){

    for(size_t p_sq_cm = 0; p_sq_cm < 1; p_sq_cm++){
    // run over all momenta squared (back-to-back hardcoded) and gamma 
    // combinations
    for(size_t p_sq_1 = 0; p_sq_1 < nb_mom_sq; p_sq_1++){
      size_t p_sq_2 = p_sq_1;
//    for(size_t p_sq_2 = 0; p_sq_2 < nb_mom_sq; p_sq_2++){
    for(size_t p_sq_3 = 0; p_sq_3 < nb_mom_sq; p_sq_3++){
      size_t p_sq_4 = p_sq_3;
//    for(size_t p_sq_4 = 0; p_sq_4 < nb_mom_sq; p_sq_4++){

      // index for access of element
//      size_t i = p_sq*nb_dg*nb_dg + so*nb_dg + si;

//      //only diagonal elements
//      size_t p_sq_so = p_sq_1;
//      size_t p_sq_si = p_sq;


      // loop over op and set index pairs
      for(auto& el_1 : op_Corr){
        if(el_1.gamma[0] == dg[so]){
        if((el_1.p3[0]*el_1.p3[0] + el_1.p3[1]*el_1.p3[1] + 
            el_1.p3[2]*el_1.p3[2]) == p_sq_1){ 

        for(auto& el_2 : op_Corr){
          if(el_2.gamma[0] == dg[so]){
          if((el_2.p3[0]*el_2.p3[0] + el_2.p3[1]*el_2.p3[1] + 
              el_2.p3[2]*el_2.p3[2]) == p_sq_2){ 

          if((el_1.p3[0]+el_2.p3[0])*(el_1.p3[0]+el_2.p3[0]) + 
             (el_1.p3[1]+el_2.p3[1])*(el_1.p3[1]+el_2.p3[1]) +
             (el_1.p3[2]+el_2.p3[2])*(el_1.p3[2]+el_2.p3[2]) == p_sq_cm){


          size_t id1 = el_1.id;
          // thats the generalized version of nb_mom - p - 1 including 
          // a faster running gamma structure
//          size_t id2 = nb_op - nb_dg * (id1/nb_dg + 1) + so;
          size_t id2 = el_2.id;


          for(auto& el_3 : op_Corr){
            if(el_3.gamma[0] == dg[si]){
            if((el_3.p3[0]*el_3.p3[0] + el_3.p3[1]*el_3.p3[1] + 
                el_3.p3[2]*el_3.p3[2]) == p_sq_3){ 
    
            for(auto& el_4 : op_Corr){
              if(el_4.gamma[0] == dg[si]){
              if((el_4.p3[0]*el_4.p3[0] + el_4.p3[1]*el_4.p3[1] + 
                  el_4.p3[2]*el_4.p3[2]) == p_sq_4){ 
    
              if((el_3.p3[0]+el_4.p3[0])*(el_3.p3[0]+el_4.p3[0]) + 
                 (el_3.p3[1]+el_4.p3[1])*(el_3.p3[1]+el_4.p3[1]) +
                 (el_3.p3[2]+el_4.p3[2])*(el_3.p3[2]+el_4.p3[2]) == p_sq_cm){
    
    
              size_t id3 = el_3.id;
              // thats the generalized version of nb_mom - p - 1 including 
              // a faster running gamma structure
    //          size_t id2 = nb_op - nb_dg * (id1/nb_dg + 1) + so;
              size_t id4 = el_4.id;

//          for(auto& el_si : op_Corr)
//            if((el_si.p3[0]*el_si.p3[0] + el_si.p3[1]*el_si.p3[1] + 
//                el_si.p3[2]*el_si.p3[2]) == p_sq_si){ 
//            if(el_si.gamma[0] == dg[si]){
//              size_t id3 = el_si.id;
//              size_t id4 = nb_op - nb_dg * (id3/nb_dg + 1) + si;

              // save p^2 and gamma structure at source and sink
              op_C4[i].p_sq_cm = p_sq_cm;
              op_C4[i].p_sq_so_1 = p_sq_1;
              op_C4[i].p_sq_so_2 = p_sq_2;
              op_C4[i].p_sq_si_1 = p_sq_3;
              op_C4[i].p_sq_si_2 = p_sq_4;
              op_C4[i].dg_so = so;
              op_C4[i].dg_si = si;

              op_C4[i].index.emplace_back(
                  std::array<size_t, 4>{{id1, id2, id3, id4}});

            }// if over p_sq_cm
            }}}//loops over particle 4
          }}}//loops over particle 3
        }// if over p_sq_cm
        }}}//loops over particle 2
      }}}//loops over particle 1

      //TODO: there are empty op_C4[i] like this
      i++;
      j++;
      
    }}//loops over displ-gamma
//  }}
  }}}//loop over mom_sq

//  if(j != op_C4.size()){
//    std::cout << "Error in LapH::set_C4(): nb_op not equal to allocated "
//                 "number of operators" << std::endl;
//    exit(0);
//  } 

}
// *****************************************************************************
// *****************************************************************************
// *****************************************************************************
static std::array<int, 3> create_3darray_from_string(std::string in) { 

  std::array<int, 3> out;
  std::vector<std::string> tokens;
  // erasing the brakets at the beginning and the end
  in.erase(0,2);
  in.erase(in.end()-1);

  boost::split(tokens, in, boost::is_any_of(","));

  return {boost::lexical_cast<int>(tokens[0]),
          boost::lexical_cast<int>(tokens[1]),
          boost::lexical_cast<int>(tokens[2]) };

}
// *****************************************************************************
static void create_all_momentum_combinations(const std::vector<int>& in, 
                                        std::vector<std::array<int, 3> >& out) {
  // creating all momentum combinations possible and needed
  int max_p = sqrt(*std::max_element(in.begin(), in.end()));
  std::vector<std::array<int, 3> > all_p;
  for(int p1 = -max_p; p1 < max_p+1; p1++)
    for(int p2 = -max_p; p2 < max_p+1; p2++)
      for(int p3 = -max_p; p3 < max_p+1; p3++)
        all_p.push_back({p1, p2, p3});
  // copying wanted combinations into out array
  for(const auto& p : in)
    for(const auto& all : all_p)
      if(p == all[0]*all[0] + all[1]*all[1] + all[2]*all[2])
        out.push_back(all);

}
// *****************************************************************************
static void create_mom_array_from_string(std::string in, 
                                        std::vector<std::array<int, 3> >& out) {
  // erase the p (first entry)
  in.erase(0,1);
  std::vector<std::string> tokens;
  boost::split(tokens, in, boost::is_any_of(","));
  std::vector<int> p;
  for(const auto& t : tokens)
    p.push_back(boost::lexical_cast<int>(t));

  create_all_momentum_combinations(p, out);

}
// *****************************************************************************
/// @brief Makes an operator list object from a string
Operator_list make_operator_list (const std::string& operator_string) {

  Operator_list op_list; // return object

  // Two steps are necessary: 1. Getting all operators in one list which are 
  //                             separated by ";"
  //                          2. Separating the individual operators into its
  //                             smaller bits, which are separated by "."
  // Tokenize the string on the ";" delimiter -> Individual operators
  std::vector<std::string> operator_tokens;
  boost::split(operator_tokens, operator_string, boost::is_any_of(":"));

  // running over opeator tokens and split them further (Step 2):
  for (const auto& op_t : operator_tokens){
    std::vector<std::string> tokens;
    boost::split(tokens, op_t, boost::is_any_of("."));
    for (auto str : tokens){
      std::vector<int> gammas;
      std::array<int, 3> dil_vec;
      std::vector<std::array<int, 3> > mom_vec;
      // getting the gamma structure
      if(str.compare(0,1,"g") == 0)
        gammas.push_back(boost::lexical_cast<int>(str.erase(0,1)));
      // getting the displacement indices
      else if (str.compare(0,1,"d") == 0) {
        if(str.compare(1,1,"0") == 0)
          dil_vec = {0, 0, 0};
        else if (str.compare(1,1,"(") == 0)
          dil_vec = create_3darray_from_string(str);
        else {
         std::cout << "Something wrong with the displacement in the operator" \
                      " definition" << std::endl;
         exit(0);
        }
      }
      // getting the momenta
      else if (str.compare(0,1,"p") == 0) {
        if(str.compare(1,1,"(") == 0)
          mom_vec.push_back(create_3darray_from_string(str));
        else 
          create_mom_array_from_string(str, mom_vec);
      }
      // catching wrong entries
      else {
        std::cout << "there is something wrong with the operators" << std::endl;
        exit(0);
      }
      op_list.push_back(Operators(gammas, dil_vec, mom_vec));
    }
  }
  return op_list;
}
// *****************************************************************************
/// @brief Makes an operator list object from a string
Correlator_list make_correlator_list (const std::string& correlator_string) {

  Correlator_list corr_list; // return object

  std::vector<std::string> correlator_tokens;
  boost::split(correlator_tokens, correlator_string, boost::is_any_of(":"));

  std::string type;
  std::vector<int> quark_number;
  std::vector<int> operator_number;
  std::string GEVP;
  std::vector<int> tot_mom;
  for (auto corr_t : correlator_tokens){
    // getting the type name
    if (corr_t.compare(0,1,"C") == 0)
      type = corr_t;
    // getting quark numbers
    else if (corr_t.compare(0,1,"Q") == 0) 
      quark_number.push_back(boost::lexical_cast<int>(corr_t.erase(0,1)));
    // getting operator numbers
    else if (corr_t.compare(0,2,"Op") == 0)
      operator_number.push_back(boost::lexical_cast<int>(corr_t.erase(0,2)));
    // getting the GEVP type
    else if (corr_t.compare(0,1,"G") == 0)
      GEVP = corr_t;
    // getting total momenta for moving frames
    else if (corr_t.compare(0,1,"P") == 0) {
      corr_t.erase(0,1);
      std::vector<std::string> tokens;
      boost::split(tokens, corr_t, boost::is_any_of(","));
      for(auto t : tokens)
        tot_mom.push_back(boost::lexical_cast<int>(t));
    }
    // catching wrong entries
    else {
      std::cout << "there is something wrong with the correlators" << std::endl;
      exit(0);
    }
    corr_list.push_back(Correlators(type, quark_number, operator_number, GEVP, 
                                    tot_mom));
  }
  std::cout << quark_number << std::endl;
  std::cout << operator_number << std::endl;
  std::cout << tot_mom << std::endl;

  return corr_list;
}
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void GlobalData::quark_input_data_handling (
    const std::vector<std::string> quark_configs) {
  try{
    // Transform each configured quark into a quark via make_quark, inserting each
    // object into the quark vector.
    std::transform(quark_configs.begin(), quark_configs.end(),
        std::back_inserter(quarks), make_quark);
    // checking the contents for correctness
    std::for_each(quarks.begin(), quarks.end(), quark_check);
  }
  catch(std::exception& e){
    std::cout << e.what() << "\n";
    exit(0);
  }
}
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void GlobalData::operator_input_data_handling (
    const std::vector<std::string> operator_list_configs) {
  try{
    // Transform each configured quark into a quark via make_quark, inserting each
    // object into the quark vector.
    std::transform(operator_list_configs.begin(), operator_list_configs.end(),
        std::back_inserter(operator_list), make_operator_list);
    // TODO: Test routine like for quark_input_data_handling
  }
  catch(std::exception& e){
    std::cout << "operator_input_data_handling: " << e.what() << "\n";
    exit(0);
  }
}
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void GlobalData::correlator_input_data_handling (
    const std::vector<std::string> correlator_list_configs) {
  try{
    // Transform each configured quark into a quark via make_quark, inserting each
    // object into the quark vector.
    std::transform(correlator_list_configs.begin(), 
                   correlator_list_configs.end(),
                   std::back_inserter(correlator_list), make_correlator_list);
    // TODO: Test routine like for quark_input_data_handling
  }
  catch(std::exception& e){
    std::cout << "correaltor_input_data_handling: " << e.what() << "\n";
    exit(0);
  }
}
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void GlobalData::read_parameters (int ac, char* av[]) {

  try{
    std::string input_file;
    std::string output_file;
    // Variables that will store parsed values for quarks.
    std::vector<std::string> quark_configs;

    // Declare a group of options that will be allowed only on command line
    po::options_description generic("Command line options");
    generic.add_options()("help,h", "produce help message")("version,v",
        "print version string")("verbose",
        "does additional tests and prints more details")("input,i",
        po::value<std::string>(&input_file)->default_value("LapHs.in"),
        "name of input file.")("output,o",
        po::value<std::string>(&output_file)->default_value("LapHs.out"),
        "name of output file.");

    // Declare a group of options that will be
    // allowed both on command line and in input file
    po::options_description config("Input file options");
    // lattice options
    config.add_options()("output_path",
        po::value<std::string>(&path_output)->default_value("../../contractions"),
        "path for output")
        ("config_path",
        po::value<std::string>(&path_config)->default_value("../../configs"),
        "path for configurations")
        ("lattice", po::value<std::string>(&name_lattice)->
        default_value("lattice"),"Codename of the lattice")("Lt", 
        po::value<int>(&Lt)->default_value(0),
        "Lt: temporal lattice extend")("Lx",
        po::value<int>(&Lx)->default_value(0),
        "Lx: lattice extend in x direction")("Ly",
        po::value<int>(&Ly)->default_value(0),
        "Ly: lattice extend in y direction")("Lz",
        po::value<int>(&Lz)->default_value(0),
        "Lz: lattice extend in z direction");
    // eigenvector options
    config.add_options()("number_of_eigen_vec",
        po::value<int>(&number_of_eigen_vec)->default_value(0),
        "Number of eigen vectors")("path_eigenvectors",
        po::value<std::string>(&path_eigenvectors)->default_value("."),
        "directory of eigenvectors")("name_eigenvectors",
        po::value<std::string>(&name_eigenvectors)->default_value(
            "eigenvector"),
        "name of eigenvectors\nThe full name is internally created to:\n"
            "\"name_of_eigenvectors.eigenvector\n."
            "time slice.configuration\"");
    // perambulator options
    config.add_options()("path_perambulators",
        po::value<std::string>(&path_perambulators)->default_value("."),
        "directory of perambulators")("name_perambulators",
        po::value<std::string>(&name_perambulators)->default_value(
            "perambulator"),
        "name of perambulators\nThe full name is internally created to:\n"
            "\"rather long\n."
            "t_sink.configuration\"");
    // quark options
    config.add_options()("quarks.quark", make_value(&quark_configs),
        "quark input, must be of type:\n"
            "quark = \n type:number of rnd. vec.:\n"
            " dil type time:number of dil time:\n"
            " dil type ev:number of dil ev:\n"
            " dil type Dirac:number of dil Dirac");
    // operator list options
    config.add_options()("operator_lists.operator_list", 
        make_value(&operator_list_configs),
        "operator input is rather complicated - see documentation!!");
    // correlator list options
    config.add_options()("correlator_lists.correlator_list", 
        make_value(&correlator_list_configs),
        "correlator input is rather complicated - see documentation!!");
    // configuration options
    config.add_options()("start_config",
        po::value<int>(&start_config)->default_value(-1), "First configuration")(
        "end_config", po::value<int>(&end_config)->default_value(0),
        "Last configuration")("delta_config",
        po::value<int>(&delta_config)->default_value(0),
        "Stepsize between two configurations");

    po::options_description cmdline_options;
    cmdline_options.add(generic).add(config);

    po::options_description input_file_options;
    input_file_options.add(config);

    po::options_description visible("Allowed options");
    visible.add(generic).add(config);
    po::positional_options_description p;
    p.add("input-file", -1);

    po::variables_map vm;
    po::store(po::command_line_parser(ac, av).options(cmdline_options).
                                              positional(p).run(),vm);
    po::notify(vm);

    // command line options ****************************************************
    if(vm.count("help")){
      std::cout << visible << "\n";
      exit(0);
    }
    if(vm.count("verbose")){
      verbose = 1;
    }
    else verbose = 0;
    if(vm.count("version")){
      std::cout << "stochastic LapH code, version under construction \n";
      exit(0);
    }
    std::ifstream ifs(input_file.c_str());
    if(!ifs){
      std::cout << "CANNOT open input file: " << input_file << "\n";
      exit(0);
    }
    else{
      po::store(parse_config_file(ifs, input_file_options), vm);
      po::notify(vm);
    }
    ifs.close();

    // input file options ******************************************************
    //
    lattice_input_data_handling(path_output, name_lattice, path_config, 
                                Lt, Lx, Ly, Lz);
    //
    eigenvec_perambulator_input_data_handling(number_of_eigen_vec,
        path_eigenvectors, name_eigenvectors, path_perambulators,
        name_perambulators);
    //
    quark_input_data_handling(quark_configs);
    //
    operator_input_data_handling(operator_list_configs);
    //
    correlator_input_data_handling(correlator_list_configs);
    //
    config_input_data_handling(start_config, end_config, delta_config);

    // computing some global variables depending on the input values ***********
    dim_row = Lx * Ly * Lz * 3;

    //needed for config_utils.h
    //4 is number of directions, 3 number of colors and 2 factor
    //for memory requirement of complex numbers
    V_TS = dim_row * 4 * 3 * 2;
    V_for_lime = V_TS * Lt;

    //dirac structure hard coded
    dg.push_back(5);

    //displacement not supported yet
    number_of_displ_gamma = dg.size();
    number_of_displ = 1;
    number_of_dirac = dg.size();
    number_of_VdaggerV = (momentum_squared.size()/2+1)*number_of_displ;
    number_of_rVdaggerVr = momentum_squared.size()*number_of_displ;
    number_of_operators = momentum_squared.size() * number_of_displ_gamma;
    number_of_momentum_squared = number_of_max_mom + 1;

    init_from_infile();

  }
  catch(std::exception& e){
    std::cout << e.what() << "\n";
    exit(0);
  }
}
