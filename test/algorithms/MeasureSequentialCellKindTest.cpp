
#include "open_char.h"

#include "Context.h"
#include "Template.h"
#include "Algorithms.h"

#include "TestUtils.h"


using namespace open_char;

void test_dff_ckb_rb_sb(Context &ctx, Algorithms &algs)
{
    CREATE_DFF_CKB_SB_RB_CELL(ctx, c1);

    RUN_SIMULATIONS(ctx, algs.PrepareSeqCellKindSims(c1));
    algs.MeasureSeqCellKind(c1);

    assert(c1.GetSequential().GetKind() == SequentialKind::FLIP_FLOP);
}

void test_lat_ckb_r_s(Context &ctx, Algorithms &algs)
{
    CREATE_LAT_CKB_R_S_CELL(ctx, c1);
    RUN_SIMULATIONS(ctx, algs.PrepareSeqCellKindSims(c1));
    algs.MeasureSeqCellKind(c1);

    assert(c1.GetSequential().GetKind() == SequentialKind::LATCH);
    assert(c1.GetSequential().GetEnablePolarity() == 0);
}

int main()
{
    ALG_TEST_INIT(ctx, algs);

    ctx.AddNetlist(TEST_NETLIST_DIR "/basic_gates.cdl");

    test_dff_ckb_rb_sb  (ctx, algs);
    test_lat_ckb_r_s    (ctx, algs);

    return 0;
}