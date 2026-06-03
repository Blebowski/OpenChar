
#include "open_char.h"

#include "Context.h"
#include "Template.h"
#include "Algorithms.h"

#include "TestUtils.h"


using namespace open_char;

void test_dff_ckb_rb_sb(Context &ctx, Algorithms &algs)
{
    CREATE_DFF_CKB_SB_RB_CELL(ctx, c1);
    c1.GetSequential().SetKind(SequentialKind::FLIP_FLOP);
    c1.GetPin("RB").SetPolarity(0);
    c1.GetPin("SB").SetPolarity(0);
    c1.GetSequential().SetClockPin(&(c1.GetPin("CKB")));
    c1.GetSequential().SetClockPolarity(EdgeKind::FALLING);

    Template t("MY_TEMP");
    t.SetKind(TemplateKind::CONSTRAINT);

    t.AddIndex1(0.05);
    t.AddIndex1(0.2);

    t.AddIndex2(0.05);
    t.AddIndex2(0.3);

    c1.SetConstraintTemplate(&t);

    ctx.GetSimulationPool().Start();
    algs.PrepareSetupSims(c1);

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (!c1.IsSimulationFinished()) {
            continue;
        }
        auto [all_setups_measured, sims_ok] = algs.MeasureSetup(c1);
        assert(sims_ok);
        if (all_setups_measured) {
            break;
        }
    }
    ctx.GetSimulationPool().WaitDone();

    // fall_constraint() {
    //       index_1 ("0.020000, 0.100000")
    //       index_2 ("0.020000, 0.100000")
    //       values (
    //         "0.016865, -0.021671"
    //         "0.072623, 0.011918"
    //       ) ;
    //     } /* end fall_constraint */

    Pin& pin = c1.GetPin("D");
    Arc& arc = pin.GetArcs()[0];

    EQUAL_WITH_TOL(arc.GetFallConstraints()[0][0], 0.016865);
    EQUAL_WITH_TOL(arc.GetFallConstraints()[0][1], -0.021671);
    EQUAL_WITH_TOL(arc.GetFallConstraints()[1][0], 0.072623);
    EQUAL_WITH_TOL(arc.GetFallConstraints()[1][1], 0.011918);
}

int main()
{
    ALG_TEST_INIT(ctx, algs);

    ctx.AddNetlist(TEST_NETLIST_DIR "/basic_gates.cdl");

    test_dff_ckb_rb_sb  (ctx, algs);

    return 0;
}