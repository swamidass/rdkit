//
//  Copyright (c) 2019 Greg Landrum
//
//   @@ All Rights Reserved @@
//  This file is part of the RDKit.
//  The contents are covered by the terms of the BSD license
//  which is included in the file license.txt, found at the root
//  of the RDKit source tree.
///
#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do
                           // this in one cpp file
#include "catch.hpp"

#include <GraphMol/RDKitBase.h>
#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/SmilesParse/SmilesWrite.h>
#include <GraphMol/ChemTransforms/ChemTransforms.h>
#include <GraphMol/FileParsers/FileParsers.h>
#include <GraphMol/FileParsers/MolSupplier.h>
#include <GraphMol/Substruct/SubstructMatch.h>

using namespace RDKit;
using std::unique_ptr;

TEST_CASE("Github #1039", "[]") {
  SECTION("double bond") {
    auto m1 = "C/C=C/C=C/C"_smiles;
    REQUIRE(m1);
    std::vector<unsigned int> bonds = {2};
    std::unique_ptr<ROMol> pieces(MolFragmenter::fragmentOnBonds(*m1, bonds));
    REQUIRE(pieces);
    CHECK(pieces->getNumAtoms() == 8);
    REQUIRE(pieces->getBondBetweenAtoms(3, 6));
    REQUIRE(pieces->getBondBetweenAtoms(2, 7));
    CHECK(pieces->getBondBetweenAtoms(3, 6)->getBondType() == Bond::SINGLE);
    CHECK(pieces->getBondBetweenAtoms(3, 6)->getBondDir() == Bond::ENDUPRIGHT);
    CHECK(pieces->getBondBetweenAtoms(2, 7)->getBondType() == Bond::SINGLE);
    CHECK(pieces->getBondBetweenAtoms(2, 7)->getBondDir() == Bond::ENDUPRIGHT);
    CHECK(MolToSmiles(*pieces) == "[2*]/C=C/C.[3*]/C=C/C");
  }
  SECTION("atomic stereo") {
    auto m1 = "C(C)(F)(Cl)O"_smiles;
    REQUIRE(m1);
    m1->getBondWithIdx(0)->setBondDir(Bond::BEGINWEDGE);
    std::vector<unsigned int> bonds = {0};
    std::unique_ptr<ROMol> pieces(MolFragmenter::fragmentOnBonds(*m1, bonds));
    REQUIRE(pieces);
    CHECK(pieces->getNumAtoms() == 7);
    REQUIRE(pieces->getBondBetweenAtoms(0, 6));
    REQUIRE(pieces->getBondBetweenAtoms(1, 5));
    CHECK(pieces->getBondBetweenAtoms(0, 6)->getBondDir() == Bond::BEGINWEDGE);
    CHECK(pieces->getBondBetweenAtoms(1, 5)->getBondDir() == Bond::NONE);
    // no actual stereo in the SMILES here since we haven't assigned it (need a
    // conformer to do that using wedging)
    CHECK(MolToSmiles(*pieces) == "*C.[1*]C(O)(F)Cl");
  }
}

TEST_CASE("molzip", "[]") {
    SECTION("basic tests") {
        auto a = "C[*:1]"_smiles;
        auto b = "N[*:1]"_smiles;
        auto mol = molzip(*a,*b);
        CHECK(MolToSmiles(*mol) == "CN");
    }
    {
        auto a = "[C@H](Br)([*:1])F"_smiles;
        auto b = "[*:1]N"_smiles;
        auto mol = molzip(*a,*b);
        CHECK(MolToSmiles(*mol) == "N[C@H](F)Br");
        MolzipParams p;
        p.preserveChirality = true;
        mol = molzip(*a, *b, p);
        CHECK(MolToSmiles(*mol) == "N[C@@H](F)Br");
    }
    {
        auto a = "[C@H]([*:1])(Br)F"_smiles;
        auto b = "[*:1]N"_smiles;
        auto mol = molzip(*a,*b);
        CHECK(MolToSmiles(*mol) == "N[C@H](F)Br");
        MolzipParams p;
        p.preserveChirality = true;
        mol = molzip(*a, *b, p);
        CHECK(MolToSmiles(*mol) == "N[C@H](F)Br");
    }
    
    {
           auto a = "[C@H]([*:1])(Br)([*:2])"_smiles;
           auto b = "[*:1]N.[*:2]Br"_smiles;
           auto mol = molzip(*a,*b);
           CHECK(MolToSmiles(*mol) == "N[C@H](F)Br");
           MolzipParams p;
           p.preserveChirality = true;
           mol = molzip(*a, *b, p);
           CHECK(MolToSmiles(*mol) == "N[C@H](F)Br");
       }
}
