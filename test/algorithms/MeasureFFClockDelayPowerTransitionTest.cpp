
#include "open_char.h"

#include "Context.h"
#include "Template.h"
#include "Algorithms.h"

#include "TestUtils.h"


using namespace open_char;

static void test_dff_ckb_rb_sb(Context &ctx, Algorithms &algs)
{
    CREATE_DFF_CKB_SB_RB_CELL(ctx, c1);
    c1.GetSequential().SetKind(SequentialKind::FLIP_FLOP);
    c1.GetPin("RB").SetPolarity(0);
    c1.GetPin("SB").SetPolarity(0);
    c1.GetSequential().SetClockPin(&(c1.GetPin("CKB")));
    c1.GetSequential().SetClockPolarity(EdgeKind::FALLING);

    Template t("MY_TEMP");
    t.SetKind(TemplateKind::DELAY);

    t.AddIndex1(0.01);
    t.AddIndex1(0.1);

    t.AddIndex2(0.02);
    t.AddIndex2(0.3);

    c1.SetDelayTemplate(&t);

    RUN_SIMULATIONS(ctx, algs.PrepareFFClockDelayTransitionPowerSims(c1));
    assert(algs.MeasureFFClockDelaysTransitionsPowers(c1));

    Pin& pin = c1.GetPin("Q");
    Arc& arc = pin.GetArcs()[0];

    // cell_rise() {
    //   index_1 ("0.010000, 0.100000")
    //   index_2 ("0.020000, 0.300000")
    //   values (
    //     "0.128000, 0.702000"
    //     "0.137000, 0.711000"
    //   ) ;
    // }

    CHECK_FLOAT(arc.GetRiseDelays()[0][0], 0.128000);
    CHECK_FLOAT(arc.GetRiseDelays()[0][1], 0.702000);
    CHECK_FLOAT(arc.GetRiseDelays()[1][0], 0.137000);
    CHECK_FLOAT(arc.GetRiseDelays()[1][1], 0.711000);

    // rise_transition() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //      "0.077000, 0.988000"
    //      "0.076000, 0.988000"
    //     ) ;
    // } /* end rise_transition */

    CHECK_FLOAT(arc.GetRiseTransitions()[0][0], 0.077000);
    CHECK_FLOAT(arc.GetRiseTransitions()[0][1], 0.988000);
    CHECK_FLOAT(arc.GetRiseTransitions()[1][0], 0.076000);
    CHECK_FLOAT(arc.GetRiseTransitions()[1][1], 0.988000);

    // cell_fall() {
    //   index_1 ("0.010000, 0.100000")
    //   index_2 ("0.020000, 0.300000")
    //   values (
    //      "0.122000, 0.702000"
    //      "0.131000, 0.711000"
    //   ) ;
    // } /* end cell_fall */

    CHECK_FLOAT(arc.GetFallDelays()[0][0], 0.122000);
    CHECK_FLOAT(arc.GetFallDelays()[0][1], 0.702000);
    CHECK_FLOAT(arc.GetFallDelays()[1][0], 0.131000);
    CHECK_FLOAT(arc.GetFallDelays()[1][1], 0.711000);

    // fall_transition() {
    //   index_1 ("0.010000, 0.100000")
    //   index_2 ("0.020000, 0.300000")
    //   values (
    //      "0.072000, 0.947000"
    //      "0.072000, 0.948000"
    //   ) ;
    // } /* end fall_transition */

    CHECK_FLOAT(arc.GetFallTransitions()[0][0], 0.072000);
    CHECK_FLOAT(arc.GetFallTransitions()[0][1], 0.947000);
    CHECK_FLOAT(arc.GetFallTransitions()[1][0], 0.072000);
    CHECK_FLOAT(arc.GetFallTransitions()[1][1], 0.948000);

    // rise_power() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //      "0.020281, 0.020193"
    //      "0.021776, 0.021686"
    //     ) ;
    // } /* end rise_power */

    CHECK_FLOAT(arc.GetRisePowers()[0][0], 0.020281);
    CHECK_FLOAT(arc.GetRisePowers()[0][1], 0.020193);
    CHECK_FLOAT(arc.GetRisePowers()[1][0], 0.021776);
    CHECK_FLOAT(arc.GetRisePowers()[1][1], 0.021686);

    // fall_power() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //      "0.046226, 0.445298"
    //      "0.047759, 0.446794"
    //     ) ;
    // } /* end fall_power */

    // TODO: Crosscheck if rise power and fall power are not swapped !

    CHECK_FLOAT(arc.GetFallPowers()[0][0], 0.046226);
    CHECK_FLOAT(arc.GetFallPowers()[0][1], 0.445298);
    CHECK_FLOAT(arc.GetFallPowers()[1][0], 0.047759);
    CHECK_FLOAT(arc.GetFallPowers()[1][1], 0.446794);
}

int main()
{
    ALG_TEST_INIT(ctx, algs);

    ctx.AddNetlist(TEST_NETLIST_DIR "/basic_gates.cdl");

    test_dff_ckb_rb_sb(ctx, algs);

    return 0;
}