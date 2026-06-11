
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
    c1.GetSequential().SetClockPin(&(c1.GetPin("CKB")));
    c1.GetSequential().SetClockPolarity(EdgeKind::FALLING);

    Template t("MY_TEMP");
    t.SetKind(TemplKind::CONSTRAINT);

    t.AddIndex1(0.05);
    t.AddIndex1(0.2);

    t.AddIndex2(0.05);
    t.AddIndex2(0.3);

    c1.SetConstraintTemplate(&t);

    ctx.GetSimulationPool().Start();
    algs.PrepareFFSetupOrHoldSims(c1, ArcKind::SEQ_HOLD);

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (!c1.IsSimulationFinished()) {
            continue;
        }
        auto [all_setups_measured, sims_ok] = algs.MeasureFFSetupOrHold(c1, ArcKind::SEQ_HOLD);
        assert(sims_ok);
        if (all_setups_measured) {
            break;
        }
    }
    ctx.GetSimulationPool().WaitDone();

    // fall_constraint() {
    //     index_1 ("0.050000, 0.200000")
    //     index_2 ("0.050000, 0.300000")
    //     values (
    //     "-0.036123, 0.110197"
    //     "-0.270233, -0.133588"
    //     ) ;
    // } /* end fall_constraint */

    Pin& pin = c1.GetPin("D");
    Arc& arc = pin.GetArcs()[0];

    CHECK_FLOAT(arc.GetFallConstraints()[0][0], -0.036123);
    CHECK_FLOAT(arc.GetFallConstraints()[0][1], 0.110197);
    CHECK_FLOAT(arc.GetFallConstraints()[1][0], -0.270233);
    CHECK_FLOAT(arc.GetFallConstraints()[1][1], -0.133588);
}

int main()
{
    ALG_TEST_INIT(ctx, algs);

    ctx.AddNetlist(TEST_NETLIST_DIR "/basic_gates.cdl");

    test_dff_ckb_rb_sb  (ctx, algs);

    return 0;
}