
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
    t.SetKind(TemplateKind::DELAY);

    t.AddIndex1(0.01);
    t.AddIndex1(0.1);

    t.AddIndex2(0.02);
    t.AddIndex2(0.3);

    c1.SetDelayTemplate(&t);

    RUN_SIMULATIONS(ctx, algs.PrepareFFClockDelaySims(c1));
    algs.MeasureFFClockDelay(c1);
    algs.MeasureFFClockTransition(c1);
    algs.MeasureFFClockPowers(c1);

    Pin& pin = c1.GetPin("Q");
    Arc& arc = pin.GetArcs()[0];

    // cell_rise() {
    //   index_1 ("0.010000, 0.100000")
    //   index_2 ("0.020000, 0.300000")
    //   values (
    //     "0.127000, 0.702500"
    //     "0.136600, 0.712000"
    //   ) ;
    // }

    EQUAL_WITH_TOL(arc.GetRiseDelays()[0][0], 0.127000);
    EQUAL_WITH_TOL(arc.GetRiseDelays()[0][1], 0.702500);
    EQUAL_WITH_TOL(arc.GetRiseDelays()[1][0], 0.136600);
    EQUAL_WITH_TOL(arc.GetRiseDelays()[1][1], 0.712000);

    // rise_transition() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //       "0.075200, 0.986800"
    //       "0.075100, 0.986700"
    //     ) ;
    // } /* end rise_transition */

    EQUAL_WITH_TOL(arc.GetRiseTransitions()[0][0], 0.075200);
    EQUAL_WITH_TOL(arc.GetRiseTransitions()[0][1], 0.986800);
    EQUAL_WITH_TOL(arc.GetRiseTransitions()[1][0], 0.075100);
    EQUAL_WITH_TOL(arc.GetRiseTransitions()[1][1], 0.986700);

    // cell_fall() {
    //   index_1 ("0.010000, 0.100000")
    //   index_2 ("0.020000, 0.300000")
    //   values (
    //     "0.121800, 0.702500"
    //     "0.131300, 0.711900"
    //   ) ;
    // } /* end cell_fall */

    EQUAL_WITH_TOL(arc.GetFallDelays()[0][0], 0.121800);
    EQUAL_WITH_TOL(arc.GetFallDelays()[0][1], 0.702500);
    EQUAL_WITH_TOL(arc.GetFallDelays()[1][0], 0.131300);
    EQUAL_WITH_TOL(arc.GetFallDelays()[1][1], 0.711900);

    // fall_transition() {
    //   index_1 ("0.010000, 0.100000")
    //   index_2 ("0.020000, 0.300000")
    //   values (
    //     "0.072200, 0.947200"
    //     "0.072300, 0.947300"
    //   ) ;
    // } /* end fall_transition */

    EQUAL_WITH_TOL(arc.GetFallTransitions()[0][0], 0.072200);
    EQUAL_WITH_TOL(arc.GetFallTransitions()[0][1], 0.947200);
    EQUAL_WITH_TOL(arc.GetFallTransitions()[1][0], 0.072300);
    EQUAL_WITH_TOL(arc.GetFallTransitions()[1][1], 0.947300);

    // rise_power() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.019818, 0.019733"
    //     "0.021225, 0.021129"
    //     ) ;
    // } /* end rise_power */

    EQUAL_WITH_TOL(arc.GetRisePowers()[0][0], 0.019818);
    EQUAL_WITH_TOL(arc.GetRisePowers()[0][1], 0.019733);
    EQUAL_WITH_TOL(arc.GetRisePowers()[1][0], 0.021225);
    EQUAL_WITH_TOL(arc.GetRisePowers()[1][1], 0.021129);

    // fall_power() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.045777, 0.444877"
    //     "0.047278, 0.446323"
    //     ) ;
    // } /* end fall_power */

    // TODO: Crosscheck if rise power and fall power are not swapped !

    EQUAL_WITH_TOL(arc.GetRisePowers()[0][0], 0.045777);
    EQUAL_WITH_TOL(arc.GetRisePowers()[0][1], 0.444877);
    EQUAL_WITH_TOL(arc.GetRisePowers()[1][0], 0.047278);
    EQUAL_WITH_TOL(arc.GetRisePowers()[1][1], 0.446323);
}

int main()
{
    ALG_TEST_INIT(ctx, algs);

    ctx.AddNetlist(TEST_NETLIST_DIR "/basic_gates.cdl");

    test_dff_ckb_rb_sb(ctx, algs);

    return 0;
}