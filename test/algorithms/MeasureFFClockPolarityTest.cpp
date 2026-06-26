////////////////////////////////////////////////////////////////////////////////////////////////////
// OpenChar - VLSI library characterizer
// Copyright (C) 2026  Ondrej Ille
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see
// <https://www.gnu.org/licenses/>.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "open_char.h"

#include "Context.h"
#include "Template.h"
#include "Algorithms.h"

#include "TestUtils.h"


using namespace open_char;

static void test_dff_ckb_rb_sb(Context &ctx, Algorithms &algs)
{
    CREATE_DFF_CKB_SB_RB_CELL(ctx, c1);
    c1.GetSequential().SetKind(SeqKind::FLIP_FLOP);
    c1.GetPin("RB").SetPolarity(0);
    c1.GetPin("SB").SetPolarity(0);

    RUN_SIMULATIONS(ctx, algs.PrepareFFClockPolaritySims(c1));
    assert(algs.MeasureFFClockPolarity(c1));

    assert(c1.GetSequential().GetClockPolarity() == EdgeKind::FALLING);
}

static void test_dff_ck_rb_sb(Context &ctx, Algorithms &algs)
{
    CREATE_DFF_CK_SB_RB_CELL(ctx, c1);
    c1.GetSequential().SetKind(SeqKind::FLIP_FLOP);
    c1.GetPin("RB").SetPolarity(0);
    c1.GetPin("SB").SetPolarity(0);

    RUN_SIMULATIONS(ctx, algs.PrepareFFClockPolaritySims(c1));
    assert(algs.MeasureFFClockPolarity(c1));

    assert(c1.GetSequential().GetClockPolarity() == EdgeKind::RISING);
}

int main()
{
    ALG_TEST_INIT(ctx, algs);

    ctx.AddNetlist(TEST_NETLIST_DIR "/basic_gates.cdl");

    test_dff_ckb_rb_sb  (ctx, algs);
    test_dff_ckb_rb_sb  (ctx, algs);

    return 0;
}