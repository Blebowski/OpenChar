

#include <bit>
#include <thread>
#include <chrono>

#include "Algorithms.h"
#include "Context.h"
#include "Arc.h"
#include "Library.h"
#include "Simulation.h"
#include "Sequential.h"
#include "Template.h"
#include "Utils.h"

namespace open_char {

Algorithms::Algorithms(Context *ctx) :
    ctx_(ctx)
{}

int Algorithms::ToLogic(Volt val)
{
    OpCond& op_cond = ctx_->GetLibrary().GetOpCond();
    if (abs(val - op_cond.GetSupply()->GetVddVoltage()) < 0.01)
        return 1;
    return 0;
}

int Algorithms::GetBit(int64_t v, size_t index)
{
    return (v >> index) & 0x1;
}

Simulation *Algorithms::NewSimulation(std::string name, SimulationKind kind, Cell *cell)
{
    Simulation *sim = new Simulation(ctx_, name, cell, kind);

    sim->SetTimeStep(ctx_->GetVariables().GetDoubleVariable("simulation_timestep"));

    OpCond &op_cond = ctx_->GetLibrary().GetOpCond();
    sim->SetTemp(op_cond.GetTemperature());
    sim->SetSupply(op_cond.GetSupply());

    for (const auto & netlist : ctx_->GetNetlists())
        sim->AddInclude(netlist);

    for (const auto & model : ctx_->GetModels())
        sim->AddModel(model);

    return sim;
}

void Algorithms::PrepareSanitySim(Cell &cell)
{
    Template *templ = cell.GetDelayTemplate();
    assert(templ != nullptr);

    std::string sim_name = "SANITY";
    Simulation *sim = NewSimulation(sim_name, SimulationKind::TRAN, &cell);

    Supply *supply = ctx_->GetLibrary().GetOpCond().GetSupply();
    Volt log0_v = supply->GetGndVoltage();
    Volt log1_v = supply->GetVddVoltage();

    NanoSecond t_offset = 0.1;
    NanoSecond t_rise = templ->GetIndex1()[0];

    // Toggle all device pins at once with fastest possible input transition.
    // This generates total rubbish and potentially conflicting Set/Reset drivers,
    // Hopefully this is reasonable way to try if we invoke som convergence issues!
    for (auto & i_pin : cell.GetPins(PinDirection::IN)) {
        sim->AddStimuli(&i_pin, Stimulus(log0_v, log1_v, t_offset, t_rise, t_rise,
                                         2 * t_rise, 10, 1));
    }

    // TODO: Make the simulation duration minimal necessary!
    sim->SetDuration(1.0);
    cell.AddSimulation(sim);
    ctx_->GetSimulationPool().EnqueueSimulation(sim);
}

bool Algorithms::CheckSanitySim(Cell &cell)
{
    for (Simulation *sim : cell.GetSimulations()) {
        if (!sim->name_.starts_with("SANITY")) {
            continue;
        }
        return sim->CheckSucesfull();
    }
    return false;
}

void Algorithms::PrepareInputCapSims(Cell &cell)
{
    size_t i_pin_index = 0;
    for (auto & i_pin : cell.GetPins(PinDirection::IN)) {

        std::string sim_name = sprintf("ICAP_%s", i_pin.name_);
        Simulation *sim = NewSimulation(sim_name, SimulationKind::TRAN, &cell);

        Supply *supply = ctx_->GetLibrary().GetOpCond().GetSupply();
        Volt log0_v = supply->GetGndVoltage();
        Volt log1_v = supply->GetVddVoltage();

        for (auto & i_pin_2 : cell.GetPins(PinDirection::IN)) {
            if (i_pin_2.name_ == i_pin.name_) {
                sim->AddStimuli(&i_pin_2, Stimulus(log0_v, log1_v, 1, 1, 1, 2, 10, 1));
            } else {
                sim->AddStimuli(&i_pin_2, Stimulus(log0_v));
            }
        }

        sim->SetDuration(6);
        sim->PutMetaData(static_cast<int>(i_pin_index));

        i_pin.AddSimulation(sim);
        ctx_->GetSimulationPool().EnqueueSimulation(sim);

        i_pin_index++;
    }
}

bool Algorithms::MeasureInputCap(Cell &cell)
{
    for (auto & i_pin : cell.GetPins(PinDirection::IN)) {
        for (Simulation *sim : i_pin.GetSimulations()) {

            // TODO: Better approach for filtering!
            if (!sim->name_.starts_with("ICAP_")) {
                continue;
            }

            if (!sim->CheckSucesfull()) {
                return false;
            }

            Waves w = sim->ReadWaves();

            auto & pin_current = w.GetCurrent(i_pin.name_);

            // TODO: Pass time when edge starts / ends as part of simulation object
            size_t rise_start_index = w.GetIndexOfTime(1.0) + 1;
            size_t rise_end_index = w.GetIndexOfTime(2.0) - 1;

            assert (pin_current.size() > rise_start_index && pin_current.size() > rise_end_index);

            // TODO: Assumes fixed time step!
            MicroAmp i_avg = 0;
            MicroAmp i_min = 1E12;
            MicroAmp i_max = 0;

            for (size_t i = rise_start_index; i < rise_end_index; i++) {
                MicroAmp cur_abs = std::fabs(pin_current[i]);
                i_avg += cur_abs;

                if (cur_abs < i_min) {
                    i_min = cur_abs;
                }
                if (cur_abs > i_max) {
                    i_max = cur_abs;
                }
            }

            i_avg /= static_cast<double>(rise_end_index - rise_start_index);

            OpCond &op_cond = ctx_->GetLibrary().GetOpCond();
            Supply *supply = op_cond.GetSupply();
            Volt log1_v = supply->GetVddVoltage();

            // C = I / (dV/dT)
            // uA / ( V / 1 ns) = fF -> * 1E-3 to pF
            PicoFarad i_cap_rise_min = (i_min / log1_v) * 1E-3;
            PicoFarad i_cap_rise_max = (i_max / log1_v) * 1E-3;
            PicoFarad i_cap_rise_avg = (i_avg / log1_v) * 1E-3;

            i_pin.SetCapacitanceRise(i_cap_rise_min, i_cap_rise_max, i_cap_rise_avg);

            // TODO: Pass start / end times from the simulation object
            size_t fall_start_index = w.GetIndexOfTime(4.0) + 1;
            size_t fall_end_index = w.GetIndexOfTime(5.0) - 1;

            assert (pin_current.size() > fall_start_index && pin_current.size() > fall_end_index);

            // TODO: Assumes fixed time step!
            i_avg = 0;
            i_min = 1E12;
            i_max = 0;

            for (size_t i = fall_start_index; i < fall_end_index; i++) {
                MicroAmp cur_abs = std::fabs(pin_current[i]);
                i_avg += cur_abs;

                if (cur_abs < i_min) {
                    i_min = cur_abs;
                }
                if (cur_abs > i_max) {
                    i_max = cur_abs;
                }
            }
            i_avg /= static_cast<double>(fall_end_index - fall_start_index);

            // C = I / (dV/dT)
            // uA / ( V / 1 ns) = fF -> * 1E-3 to pF
            PicoFarad i_cap_fall_min = (i_min / log1_v) * 1E-3;
            PicoFarad i_cap_fall_max = (i_max / log1_v) * 1E-3;
            PicoFarad i_cap_fall_avg = (i_avg / log1_v) * 1E-3;

            i_pin.SetCapacitanceFall(i_cap_fall_min, i_cap_fall_max, i_cap_fall_avg);
        }
    }

    return true;
}

void Algorithms::PrepareComboLogicTableAndLeakageSims(Cell &cell)
{
    OpCond &op_cond = ctx_->GetLibrary().GetOpCond();
    Supply *supply = op_cond.GetSupply();
    Volt log0_v = supply->GetGndVoltage();
    Volt log1_v = supply->GetVddVoltage();

    for (auto & o_pin : cell.GetPins(PinDirection::OUT)) {

        size_t n_sims = 1;
        for (size_t i = 0; i < cell.GetPinsCount(PinDirection::IN); i++)
            n_sims *= 2;

        for (int64_t i_pin_vect = 0; i_pin_vect < static_cast<int64_t>(n_sims); i_pin_vect++) {

            std::string sim_name = "COMBO_LOGTBLLKG";
            size_t tmp = i_pin_vect;
            for (const auto & i_pin : cell.GetPins(PinDirection::IN)) {
                sim_name = sprintf("%s_%s%d", sim_name, i_pin.name_, tmp & 0x1);
                tmp >>= 1;
            }

            Simulation *sim = NewSimulation(sim_name, SimulationKind::DC, &cell);

            int i_pin_index = 0;
            for (auto & i_pin : cell.GetPins(PinDirection::IN)) {
                size_t i_pin_val = ((i_pin_vect >> i_pin_index) & 0x1);
                sim->AddStimuli((Pin*)&i_pin, Stimulus((i_pin_val == 1) ? log1_v : log0_v));

                i_pin_index++;
            }

            sim->PutMetaData(static_cast<int>(i_pin_vect));

            o_pin.AddSimulation(sim);
            ctx_->GetSimulationPool().EnqueueSimulation(sim);
        }
    }
}

bool Algorithms::MeasureComboLogicTables(Cell &cell)
{
    for (auto & o_pin : cell.GetPins(PinDirection::OUT)) {
        for (Simulation *sim : o_pin.GetSimulations()) {

            // TODO: Find better way of filtering sims
            if (!sim->name_.starts_with("COMBO_LOGTBLLKG")) {
                continue;
            }
            assert(sim->IsFinished());

            if (!sim->CheckSucesfull()) {
                return false;
            }

            Waves w = sim->ReadWaves();

            int64_t i_pin_vect = sim->GetMetaDataAt(0);
            o_pin.AddLogicTableEntry(i_pin_vect, ToLogic(w.GetVoltage(o_pin.name_)[0]));
        }
    }

    return true;
}

bool Algorithms::MeasureComboLeakage(Cell &cell)
{
    // Logic Table simulations contain all combinations of input pin states so
    // they are used to extract leakage upon each input combination.

    for (auto & o_pin : cell.GetPins(PinDirection::OUT)) {
        for (Simulation *sim : o_pin.GetSimulations()) {

            // TODO: Find better way of filtering sims
            if (!sim->name_.starts_with("COMBO_LOGTBLLKG")) {
                continue;
            }

            assert(sim->IsFinished());

            if (!sim->CheckSucesfull()) {
                return false;
            }

            Waves w = sim->ReadWaves();

            Supply *s = cell.GetLibrary()->GetOpCond().GetSupply();
            NanoWatt lkg = w.GetCurrent(s->GetVddName())[0] * s->GetVddVoltage() * 1E3;

            Expression *e = new Expression(ExpressionKind::CONSTANT, 1);
            int64_t i_pin_vect = sim->GetMetaDataAt(0);

            for (auto & i_pin : cell.GetPins(PinDirection::IN)) {
                Expression *tmp = new Expression(ExpressionKind::TERM, &(i_pin));
                if ((i_pin_vect & 0x1) == 0) {
                    tmp = new Expression(ExpressionKind::NOT, tmp);
                }
                e = new Expression(ExpressionKind::AND, e, tmp);
                i_pin_vect >>= 1;
            }

            e->Simplify();
            cell.AddLeakageTableEntry(e, lkg);
        }
        break;
    }

    return true;
}

Expression* Algorithms::ComboSumOfProducts(Cell& cell, Pin& o_pin)
{
    Expression *e = new Expression(ExpressionKind::CONSTANT, 0);

    for (auto &lt_entry : o_pin.GetLogicTable()) {
        int output = lt_entry.second;

        Expression* row_e = new Expression(ExpressionKind::CONSTANT, 1);

        if (output == 0)
            continue;

        int64_t inputs = lt_entry.first;

        int i = 0;
        for (auto &i_pin : cell.GetPins(PinDirection::IN)) {
            int val = (inputs >> i) & 0x1;

            Expression *e_rhs = new Expression(ExpressionKind::TERM, &i_pin);
            if (val == 0) {
                e_rhs = new Expression(ExpressionKind::NOT, e_rhs);
            }
            row_e = new Expression(ExpressionKind::AND, row_e, e_rhs);
            i++;
        }

        e = new Expression(ExpressionKind::OR, e, row_e);
    }

    return e;
}

Expression* Algorithms::ComboProductOfSums(Cell& cell, Pin& o_pin)
{
    // TODO: Cross-check this is correct!
    Expression *e = new Expression(ExpressionKind::CONSTANT, 1);

    for (const auto &lt_entry : o_pin.GetLogicTable()) {
        int output = lt_entry.second;

        Expression* row_e = new Expression(ExpressionKind::CONSTANT, 0);

        if (output == 1)
            continue;

        int64_t inputs = lt_entry.first;

        int i = 0;
        for (auto &i_pin : cell.GetPins(PinDirection::IN)) {
            int val = (inputs >> i) & 0x1;

            Expression *e_rhs = new Expression(ExpressionKind::TERM, &i_pin);
            if (val == 1) {
                e_rhs = new Expression(ExpressionKind::NOT, e_rhs);
            }
            row_e = new Expression(ExpressionKind::OR, row_e, e_rhs);
            i++;
        }

        e = new Expression(ExpressionKind::AND, e, row_e);
    }

    return e;
}

Expression* Algorithms::ComboRecognizeXor(Cell& cell, Pin& o_pin)
{
    size_t n_inputs = cell.GetPinsCount(PinDirection::IN);

    // Checks all rows are XOR of inputs
    for (const auto & row : o_pin.GetLogicTable()) {
        int64_t inputs = row.first;
        int output = row.second;

        int cur_out = 0;

        for (size_t i = 0; i < n_inputs; i++) {
            cur_out ^= (inputs >> i) & 0x1;
        }

        if (cur_out != output) {
            return nullptr;
        }
    }

    Expression *e = new Expression(ExpressionKind::CONSTANT, 0);

    for (auto & i_pin : cell.GetPins(PinDirection::IN)) {
        Expression *term = new Expression(ExpressionKind::TERM, &i_pin);
        e = new Expression(ExpressionKind::XOR, e, term);
    }

    return e;
}

void Algorithms::CalculateComboLogicFunctions(Cell &cell)
{
    for (auto & o_pin : cell.GetPins(PinDirection::OUT)) {
        auto &log_table = o_pin.GetLogicTable();
        assert (log_table.size() > 0);

        Expression *e = ComboRecognizeXor(cell, o_pin);

        if (e == nullptr) {
            size_t n_zeros = 0;
            size_t n_ones = 0;

            for (const auto & row : log_table) {
                assert (row.second == 0 || row.second == 1);
                if (row.second == 1)
                    n_ones++;
                if (row.second == 0)
                    n_zeros++;
            }

            if (n_ones > n_zeros) {
                e = ComboProductOfSums(cell, o_pin);
            } else {
                e = ComboSumOfProducts(cell, o_pin);
            }
        }

        e->Simplify();
        o_pin.SetLogicFunction(e);
    }
}

void Algorithms::PrepareComboDelayAndPowerSims(Cell &cell)
{
    for (auto & o_pin : cell.GetPins(PinDirection::OUT)) {

        struct test_vect {
            int64_t     in_i;
            int         out_i;
            int64_t     in_j;
            int         out_j;
        };
        std::vector<test_vect> test_vects;

        // Measure test vectors with output different and only one
        // input different
        int i = 0;
        for (auto & row_i : o_pin.GetLogicTable()) {
            int64_t row_i_in = row_i.first;
            int row_i_out = row_i.second;

            int j = 0;
            for (auto & row_j : o_pin.GetLogicTable()) {
                int64_t row_j_in = row_j.first;
                int row_j_out = row_j.second;

                if (j <= i) {
                    j++;
                    continue;
                }

                if (row_j_out == row_i_out) {
                    j++;
                    continue;
                }

                int64_t diff = row_i_in ^ row_j_in;
                if (std::popcount((unsigned long)diff) != 1) {
                    j++;
                    continue;
                }

                test_vect tv = {
                    .in_i   = row_i_in,
                    .out_i  = row_i_out,
                    .in_j   = row_j_in,
                    .out_j  = row_j_out
                };
                test_vects.push_back(tv);

                j++;
            }
            i++;
        }

        // Simulate all test vectors - Both directions
        for (const auto & tv : test_vects) {
            PrepareOneComboArcSims(o_pin, tv.in_i, tv.in_j, tv.out_i, tv.out_j);
        }
    }
}

int Algorithms::PrepareOneComboArcSims(Pin &o_pin, int64_t in_a, int64_t in_b, int out_a, int out_b)
{
    Cell *cell = o_pin.cell_;
    OpCond &op_cond = ctx_->GetLibrary().GetOpCond();
    Template *templ = cell->GetDelayTemplate();

    o_pin.AddArc(Arc(&o_pin, templ, ArcKind::COMBO, in_a, in_b, out_a, out_b));
    size_t arc_index = o_pin.GetArcs().size() - 1;

    for (size_t i = 0; i < 2; i++) {
        int64_t i_from_vect = (i == 0) ? in_a  : in_b;
        int64_t i_to_vect   = (i == 0) ? in_b  : in_a;
        int     o_from      = (i == 0) ? out_a : out_b;

        std::string prefix = "COMBO_DLYPWR";
        size_t j = 0;

        for (const auto & i_pin : cell->GetPins(PinDirection::IN)) {
            prefix = sprintf("%s_%s%d%d", prefix, i_pin.name_,
                             GetBit(i_from_vect, j), GetBit(i_to_vect, j));
            j++;
        }

        int i_tran_index = 0;
        for (NanoSecond i_tran : templ->GetIndex1()) {

            int o_cap_index = 0;
            for (const PicoFarad o_cap : templ->GetIndex2()) {

                std::string sim_name = sprintf("%s_TRAN_%f_%s_CAP_%f", prefix, i_tran, o_pin.name_, o_cap);
                Simulation *sim = NewSimulation(sim_name, SimulationKind::TRAN, cell);

                Volt log0_v = op_cond.GetSupply()->GetGndVoltage();
                Volt log1_v = op_cond.GetSupply()->GetVddVoltage();

                int i = 0;
                for (auto & i_pin : cell->GetPins(PinDirection::IN)) {

                    int i_from = GetBit(i_from_vect, i);
                    int i_to = GetBit(i_to_vect, i);

                    if (i_from == i_to) {
                        sim->AddStimuli((Pin*)&i_pin, Stimulus((i_from == 1) ? log1_v : log0_v));
                    } else {

                        assert((i_from == 0 && i_to == 1) | (i_from == 1 && i_to == 0));

                        Variables &vars = ctx_->GetVariables();

                        sim->PutMetaData(i_from);
                        sim->PutMetaData(o_from);

                        NanoSecond i_tran_cor = i_tran;
                        // Assumes linear ramp
                        if (i_from == 0) {
                            double slew_lower_rise = vars.GetDoubleVariable("slew_lower_rise");
                            double slew_upper_rise = vars.GetDoubleVariable("slew_upper_rise");
                            i_tran_cor *= (1 + slew_lower_rise + (1 - slew_upper_rise));
                        } else {
                            double slew_lower_fall = vars.GetDoubleVariable("slew_lower_fall");
                            double slew_upper_fall = vars.GetDoubleVariable("slew_upper_fall");
                            i_tran_cor *= (1 + slew_lower_fall + (1 - slew_upper_fall));
                        }

                        // TODO: Refine
                        Stimulus edge (
                            (i_from == 1) ? log1_v : log0_v,
                            (i_to == 1)   ? log1_v : log0_v,
                            1,
                            i_tran_cor,
                            i_tran_cor,
                            100,
                            100,
                            1
                        );
                        sim->AddStimuli((Pin*)&i_pin, std::move(edge));
                    }
                    i++;
                }

                sim->AddLoad(&o_pin, o_cap);

                o_pin.GetArcs()[arc_index].AddSimulation(i_tran_index, o_cap_index, sim);
                ctx_->GetSimulationPool().EnqueueSimulation(sim);

                o_cap_index++;
            }
            i_tran_index++;
        }
    }

    return 0;
}

bool Algorithms::MeasureComboDelays(Cell &cell)
{
    assert (cell.GetKind() == CellKind::COMBINATIONAL);

    Variables &vars = ctx_->GetVariables();

    double delay_in_rise = vars.GetDoubleVariable("delay_in_rise");
    double delay_in_fall = vars.GetDoubleVariable("delay_in_fall");
    double delay_out_rise = vars.GetDoubleVariable("delay_out_rise");
    double delay_out_fall = vars.GetDoubleVariable("delay_out_fall");

    Supply *supply = ctx_->GetLibrary().GetOpCond().GetSupply();
    Volt vdd_voltage = supply->GetVddVoltage();
    std::string vdd_name = supply->GetVddName();

    for (auto & o_pin : cell.GetPins(PinDirection::OUT)) {
        for (auto & arc : o_pin.GetArcs()) {

            size_t i_tran_index = 0;
            for (auto & sims_row : arc.GetSimulations()) {

                size_t o_cap_index = 0;
                for (auto & sims_row_coll : sims_row) {
                    assert(sims_row_coll.size() == 2);

                    for (Simulation *sim : sims_row_coll) {

                        // TODO: Find better way of filtering sims
                        if (!sim->name_.starts_with("COMBO_DLYPWR")) {
                            continue;
                        }

                        if (!sim->CheckSucesfull()) {
                            return false;
                        }

                        Waves w = sim->ReadWaves();
                        Pin *related_pin = arc.GetRelatedPin();

                        int i_from = sim->GetMetaDataAt(0);
                        int o_from = sim->GetMetaDataAt(1);

                        double i_th = (i_from == 0) ? delay_in_rise : delay_in_fall;
                        double o_th = (o_from == 0) ? delay_out_rise : delay_out_fall;

                        i_th *= vdd_voltage;
                        o_th *= vdd_voltage;

                        NanoSecond i_edge = w.FindTransitionTime(related_pin->name_, i_th);
                        NanoSecond o_edge = w.FindTransitionTime(o_pin.name_, o_th);

                        NanoSecond delay = o_edge - i_edge;

                        if (o_from == 0)
                            arc.SetRiseDelay(i_tran_index, o_cap_index, delay);
                        else
                            arc.SetFallDelay(i_tran_index, o_cap_index, delay);
                    }

                    o_cap_index++;
                }
                i_tran_index++;
            }
        }
    }

    return true;
}

bool Algorithms::MeasureComboTransitions(Cell &cell)
{
    assert (cell.GetKind() == CellKind::COMBINATIONAL);

    Variables &vars = ctx_->GetVariables();

    double slew_upper_rise = vars.GetDoubleVariable("slew_upper_rise");
    double slew_upper_fall = vars.GetDoubleVariable("slew_upper_fall");
    double slew_lower_rise = vars.GetDoubleVariable("slew_lower_rise");
    double slew_lower_fall = vars.GetDoubleVariable("slew_lower_fall");

    Supply *supply = ctx_->GetLibrary().GetOpCond().GetSupply();
    Volt vdd_voltage = supply->GetVddVoltage();
    std::string vdd_name = supply->GetVddName();

    for (auto & o_pin : cell.GetPins(PinDirection::OUT)) {
        for (auto & arc : o_pin.GetArcs()) {

            size_t i_tran_index = 0;
            for (auto & sims_row : arc.GetSimulations()) {

                size_t o_cap_index = 0;
                for (auto & sims_row_coll : sims_row) {
                    assert(sims_row_coll.size() == 2);

                    for (Simulation *sim : sims_row_coll) {

                        // TODO: Find better way of filtering sims
                        if (!sim->name_.starts_with("COMBO_DLYPWR")) {
                            continue;
                        }

                        if (!sim->CheckSucesfull()) {
                            return false;
                        }

                        Waves w = sim->ReadWaves();
                        int o_from = sim->GetMetaDataAt(1);

                        double lo_th = (o_from == 0) ? slew_lower_rise : slew_lower_fall;
                        double hi_th = (o_from == 0) ? slew_upper_rise : slew_upper_fall;

                        lo_th *= vdd_voltage;
                        hi_th *= vdd_voltage;

                        NanoSecond lo_time = w.FindTransitionTime(o_pin.name_, lo_th);
                        NanoSecond hi_time = w.FindTransitionTime(o_pin.name_, hi_th);

                        if (o_from == 0) {
                            arc.SetRiseTransition(i_tran_index, o_cap_index, hi_time - lo_time);
                        } else {
                            arc.SetFallTransition(i_tran_index, o_cap_index, lo_time - hi_time);
                        }
                    }
                    o_cap_index++;
                }
                i_tran_index++;
            }
        }
    }

    return true;
}

bool Algorithms::MeasureComboPowers(Cell &cell)
{
    assert (cell.GetKind() == CellKind::COMBINATIONAL);

    Supply *supply = ctx_->GetLibrary().GetOpCond().GetSupply();
    Volt vdd_voltage = supply->GetVddVoltage();
    std::string vdd_name = supply->GetVddName();

    for (auto &o_pin : cell.GetPins(PinDirection::OUT)) {
        for (auto &arc : o_pin.GetArcs()) {

            size_t i_tran_index = 0;
            for (auto & sims_row : arc.GetSimulations()) {

                size_t o_cap_index = 0;
                for (auto & sims_row_coll : sims_row) {
                    assert(sims_row_coll.size() == 2);

                    for (Simulation* sim : sims_row_coll) {

                        // TODO: Find better way of filtering sims
                        if (!sim->name_.starts_with("COMBO_DLYPWR")) {
                            continue;
                        }

                        if (!sim->CheckSucesfull()) {
                            return false;
                        }

                        Waves w = sim->ReadWaves();
                        Pin *related_pin = arc.GetRelatedPin();

                        int i_from = sim->GetMetaDataAt(0);
                        int o_from = sim->GetMetaDataAt(1);

                        std::vector<NanoWatt> pwr;
                        std::vector<MicroAmp> i_vdd = w.GetCurrent(vdd_name);
                        std::vector<MicroAmp> i_out = w.GetCurrent(o_pin.name_);

                        NanoWatt lkg = i_vdd[0] * vdd_voltage * 1E3;

                        assert (i_vdd.size() == i_out.size());

                        for (size_t i = 0; i < i_vdd.size(); i++) {

                            // The power is drain by the cell when i_vdd is negative.
                            // The power is drain by the load when i_out is positive.
                            // Sum of these therefore gives the current consumed by
                            // the cell.
                            // Also, if i_vdd is positive, it means current flows from
                            // VDD back to the supply network. We are conservative and
                            // ignore such flow as it would give more optimistic results.
                            MicroAmp i_vdd_cap = (i_vdd[i] < 0) ? i_vdd[i] : 0.0;
                            pwr.push_back(((i_vdd_cap + i_out[i]) * vdd_voltage * 1E3) - lkg);
                        }

                        // Integrate total energy between the transitions between 1 and 99 percent
                        // For "start" use the related pin, for "end" use the output pin, this spans
                        // the whole interval where power is drain due to the transition
                        Volt th_start = (i_from == 0) ? vdd_voltage * 0.01 : vdd_voltage * 0.99;
                        Volt th_end = (o_from == 0) ? vdd_voltage * 0.99 : vdd_voltage * 0.01;

                        size_t pwr_start = w.FindTransitionIndex(related_pin->name_, th_start);
                        size_t pwr_end = w.FindTransitionIndex(o_pin.name_, th_end);

                        PicoJoule e = 0;
                        NanoSecond step = sim->GetTimeStep();

                        // TODO: Time step may not need to be constant, better integrate with each step
                        //       as time diff between two consecutive steps.
                        for (size_t i = pwr_start; i < pwr_end; i++) {
                            e += pwr[i] * step;
                        }

                        // ns * nW gives us "atto Joule" -> divide to get pJ
                        e /= 1E6;
                        e = std::fabs(e);

                        if (o_from == 0) {
                            arc.SetFallPower(i_tran_index, o_cap_index, e);
                        } else {
                            arc.SetRisePower(i_tran_index, o_cap_index, e);
                        }
                    }
                    o_cap_index++;
                }
                i_tran_index++;
            }
        }
    }

    return true;
}

// Now works for D-type latch or flop. R/S or J-K flop don't work!
void Algorithms::PrepareSeqAsyncFunctionSims(Cell &cell)
{
    size_t a_pin_cnt = 0;
    for ([[maybe_unused]] auto & a_pin : cell.GetPins(PinKind::ASYNC)) {
        a_pin_cnt++;
    }

    if (a_pin_cnt == 0) {
        return;
    }

    assert(a_pin_cnt == 1 || a_pin_cnt == 2);

    size_t d_pin_cnt = 0;
    for ([[maybe_unused]] auto & i_pin : cell.GetPins(PinDirection::IN, PinKind::DATA)) {
        d_pin_cnt++;
    }

    // Support only single bit flip flops.
    assert (d_pin_cnt == 1);

    for (int64_t a_pin_vect = 0; a_pin_vect < (1 << a_pin_cnt); a_pin_vect++) {

        auto d_pin = cell.GetPins(PinDirection::IN, PinKind::DATA).front();

        for (int d = 0; d < 2; d++) {
            std::string sim_name = "SEQ_ASYNCFUNC";

            int64_t tmp = a_pin_vect;
            for (const auto & a_pin : cell.GetPins(PinKind::ASYNC)) {
                sim_name = sprintf("%s_%s%d", sim_name, a_pin.name_, tmp & 0x1);
                tmp >>= 1;
            }

            sim_name = sprintf("%s_%s%d", sim_name, d_pin.name_, d);
            Simulation *sim = NewSimulation(sim_name, SimulationKind::TRAN, &cell);

            OpCond& op_cond = ctx_->GetLibrary().GetOpCond();
            Volt log0_v = op_cond.GetSupply()->GetGndVoltage();
            Volt log1_v = op_cond.GetSupply()->GetVddVoltage();

            // Drive async pins
            tmp = a_pin_vect;
            for (auto & a_pin : cell.GetPins(PinKind::ASYNC)) {
                sim->AddStimuli((Pin*)&a_pin, Stimulus(((tmp & 0x1) == 1) ? log1_v : log0_v));
                tmp >>= 1;
            }

            // Drive data pins
            // TODO: This assumes having D-type sequential element.
            // For arbitrary one, we would need all combinations of inputs in
            // single simulation and recognize "clear candidate" and "set candidate"
            // when "no edge happened" based on output value!
            for (auto & i_pin : cell.GetPins(PinDirection::IN, PinKind::DATA)) {
                Volt v = (d == 1) ? log1_v : log0_v;
                sim->AddStimuli((Pin*)&i_pin, Stimulus(v));
            }

            // CK pulse
            for (auto & c_pin : cell.GetPins(PinKind::CLK)) {
                sim->AddStimuli((Pin*)&c_pin, Stimulus(log0_v, log1_v, 1.0, 0.01, 0.01, 1.0, 100.0, 1));
            }

            sim->PutMetaData(static_cast<int>(a_pin_vect));
            sim->PutMetaData(d);

            cell.AddSimulation(sim);
            ctx_->GetSimulationPool().EnqueueSimulation(sim);
        }
    }
}

bool Algorithms::MeasureSeqAsyncFunctions(Cell &cell)
{
    assert(cell.GetKind() == CellKind::SEQUENTIAL);

    struct a_pin_data {
        int64_t a_pin_vect;
        int  d;
        bool clr_candidate;
        bool set_candidate;
    };
    std::vector<a_pin_data> tbl;

    size_t n_set_candidates = 0;
    size_t n_clr_candidates = 0;

    for (Simulation *sim : cell.GetSimulations()) {

        // TODO: Find better way of filtering sims
        if (!sim->name_.starts_with("SEQ_ASYNCFUNC")) {
            continue;
        }

        if (!sim->CheckSucesfull()) {
            return false;
        }

        a_pin_data row = {};
        row.a_pin_vect = sim->GetMetaDataAt(0);
        row.d = sim->GetMetaDataAt(1);

        Waves w = sim->ReadWaves();
        auto q_pin = cell.GetPins(PinDirection::OUT, PinKind::DATA).front();
        Volt q_val = w.GetVoltage(q_pin.name_).back();

        if (row.d == 0 && ToLogic(q_val) == 1) {
            n_set_candidates++;
            row.set_candidate = true;
        } else if (row.d == 1 && ToLogic(q_val) == 0) {
            n_clr_candidates++;
            row.clr_candidate = true;
        }

        tbl.push_back(row);
    }

    assert( (n_set_candidates == 0 && n_clr_candidates == 1) ||
            (n_set_candidates == 1 && n_clr_candidates == 0) ||
            (n_set_candidates == 1 && n_clr_candidates == 2) ||
            (n_set_candidates == 2 && n_clr_candidates == 1));

    Expression *set_e = new Expression(ExpressionKind::CONSTANT, 0);
    Expression *clr_e = new Expression(ExpressionKind::CONSTANT, 0);

    for (auto & row : tbl) {
        if (!row.set_candidate && !row.clr_candidate) {
            continue;
        }

        assert (!(row.set_candidate && row.clr_candidate));

        size_t a_pin_vect = row.a_pin_vect;
        Expression *e = new Expression(ExpressionKind::CONSTANT, 1);

        for (auto & a_pin : cell.GetPins(PinKind::ASYNC)) {
            Expression *e2 = new Expression(ExpressionKind::TERM, &a_pin);

            if ((a_pin_vect & 0x1) == 0) {
                e2 = new Expression(ExpressionKind::NOT, e2);
            }
            e = new Expression(ExpressionKind::AND, e, e2);
            a_pin_vect >>= 1;
        }

        if (row.set_candidate) {
            set_e = new Expression(ExpressionKind::OR, set_e, e);
        }

        if (row.clr_candidate) {
            clr_e = new Expression(ExpressionKind::OR, clr_e, e);
        }
    }

    clr_e->Simplify();
    set_e->Simplify();

    // Recognize case when we have only one async pin
    if (set_e->GetKind() == ExpressionKind::CONSTANT) {
        delete set_e;
        assert(clr_e->GetKind() == ExpressionKind::NOT ||
               clr_e->GetKind() == ExpressionKind::TERM);
        cell.GetSequential().SetClear(clr_e);
    }

    if (clr_e->GetKind() == ExpressionKind::CONSTANT) {
        delete clr_e;
        assert(set_e->GetKind() == ExpressionKind::NOT ||
               set_e->GetKind() == ExpressionKind::TERM);
        cell.GetSequential().SetPreset(set_e);
    }

    // Now we should have the async pin with priority containing only
    // TERM or NOT expression based on polarity!
    // TODO: This assumes that always one async pin will have priority
    // which might not be the case since liberty allows for other
    // kinds of behavior!
    assert ((set_e->GetKind() == ExpressionKind::TERM) ||
            (set_e->GetKind() == ExpressionKind::NOT)  ||
            (clr_e->GetKind() == ExpressionKind::TERM) ||
            (clr_e->GetKind() == ExpressionKind::NOT));

    Expression *dom_e;

    if (set_e->GetKind() == ExpressionKind::TERM || set_e->GetKind() == ExpressionKind::NOT) {
        dom_e = set_e;
    } else {
        dom_e = clr_e;
    }

    // The recessive async pin still contains the condition of inactive
    // dominant pin, otherwise it would not be recognized! We substitute
    // this out with inactive value!
    if (dom_e == set_e) {
        cell.GetSequential().SetAsyncPriority(AsyncPriority::PRESET);
        if (set_e->GetKind() == ExpressionKind::NOT) {
            Pin *set_pin = set_e->GetLhs()->GetPin();
            assert(set_pin != nullptr);
            clr_e->Substitute(set_pin, 1);
        } else {
            Pin *set_pin = set_e->GetPin();
            assert(set_pin != nullptr);
            clr_e->Substitute(set_pin, 0);
        }
        clr_e->Simplify();
    } else {
        cell.GetSequential().SetAsyncPriority(AsyncPriority::CLEAR);
        if (clr_e->GetKind() == ExpressionKind::NOT) {
            Pin *clr_pin = clr_e->GetLhs()->GetPin();
            assert (clr_pin != nullptr);
            set_e->Substitute(clr_pin, 1);
        } else {
            Pin *clr_pin = clr_e->GetPin();
            assert (clr_pin != nullptr);
            set_e->Substitute(clr_pin, 0);
        }
        set_e->Simplify();
    }

    if (clr_e->GetKind() == ExpressionKind::NOT) {
        clr_e->GetLhs()->GetPin()->SetPolarity(0);
    } else {
        clr_e->GetPin()->SetPolarity(1);
    }

    if (set_e->GetKind() == ExpressionKind::NOT) {
        set_e->GetLhs()->GetPin()->SetPolarity(0);
    } else {
        set_e->GetPin()->SetPolarity(1);
    }

    cell.GetSequential().SetPreset(set_e);
    cell.GetSequential().SetClear(clr_e);

    return true;
}

void Algorithms::PrepareSeqCellKindSims(Cell &cell)
{
    for (int clock_val = 0; clock_val < 2; clock_val++) {

        Pin &c_pin = cell.GetPins(PinKind::CLK).front();

        std::string sim_name = sprintf("SEQ_EDGEVSLVL_%s%d", c_pin.name_, clock_val);
        Simulation *sim = NewSimulation(sim_name, SimulationKind::TRAN, &cell);

        OpCond& op_cond = ctx_->GetLibrary().GetOpCond();
        Volt log0_v = op_cond.GetSupply()->GetGndVoltage();
        Volt log1_v = op_cond.GetSupply()->GetVddVoltage();

        // Tie async pins to inactive value
        for (auto & a_pin : cell.GetPins(PinKind::ASYNC)) {
            sim->AddStimuli(&a_pin, Stimulus(((a_pin.GetPolarity()) == 1) ? log0_v : log1_v));
        }

        // Drive data pins to all combinations of inputs
        NanoSecond pw = 1.0;
        for (auto & i_pin : cell.GetPins(PinDirection::IN, PinKind::DATA)) {
            sim->AddStimuli(&i_pin, Stimulus(log0_v, log1_v, pw, 0.01, 0.01, pw, pw * 2, 32));
            pw *= 2;
        }

        // Scale the simulation so that we manage to try all input values!
        sim->SetDuration(pw);

        // Tie clock pin based on simulation type being performed
        for (auto & c_pin : cell.GetPins(PinKind::CLK)) {
            sim->AddStimuli(&c_pin, Stimulus((clock_val == 0) ? log0_v : log1_v));
        }

        sim->PutMetaData(clock_val);

        cell.AddSimulation(sim);
        ctx_->GetSimulationPool().EnqueueSimulation(sim);
    }
}

bool Algorithms::MeasureSeqCellKind(Cell &cell)
{
    SequentialKind seq_kind = SequentialKind::FLIP_FLOP;
    int active_clocks = 0;

    for (Simulation *sim : cell.GetSimulations()) {

        // TODO: Find better way of filtering sims from previous ones ?
        if (!sim->name_.starts_with("SEQ_EDGEVSLVL")) {
            continue;
        }

        if (!sim->CheckSucesfull()) {
            return false;
        }

        Waves w = sim->ReadWaves();

        bool has_logic_0 = false;
        bool has_logic_1 = false;

        for (auto & pin : cell.GetPins(PinDirection::OUT)) {
            for (Volt v : w.GetVoltage(pin.name_)) {
                if (ToLogic(v) == 1) {
                    has_logic_1 = true;
                } else if (ToLogic(v) == 0) {
                    has_logic_0 = true;
                }
            }
        }

        if (has_logic_0 && has_logic_1) {
            active_clocks++;
            seq_kind = SequentialKind::LATCH;
            int active_clock = sim->GetMetaDataAt(0);
            cell.GetSequential().SetEnablePolarity(active_clock);
        }
    }

    cell.GetSequential().SetKind(seq_kind);

    if (active_clocks > 1) {
        Pin &c_pin = cell.GetPins(PinKind::CLK).front();
        error("%s - Latch clock pin %s is active both at 1 and 0.", c_pin.name_);
        return false;
    }

    return true;
}

void Algorithms::PrepareFFClockPolaritySims(Cell &cell)
{
    size_t i_pin_cnt = 0;

    // TODO: Wrap calculation of number of pins!
    for ([[maybe_unused]] auto & i_pin : cell.GetPins(PinDirection::IN, PinKind::DATA)) {
        i_pin_cnt++;
    }

    for (int clock_polarity = 0; clock_polarity < 2; clock_polarity++) {

        size_t test_vect_cnt = 1 << i_pin_cnt;
        for (size_t i_pin_vect = 0 ; i_pin_vect < test_vect_cnt; i_pin_vect++) {

            std::string sim_name = sprintf("FF_CKPOL_%s%d%d", cell.GetPins(PinKind::CLK).front().name_,
                                            clock_polarity, 1 - clock_polarity);
            size_t i = 0;
            for (auto & i_pin : cell.GetPins(PinDirection::IN, PinKind::DATA)) {
                sim_name = sprintf("%s_%s%d", sim_name, i_pin.name_, (i_pin_vect >> i) & 0x1);
                i++;
            }
            Simulation *sim = NewSimulation(sim_name, SimulationKind::TRAN, &cell);

            OpCond& op_cond = ctx_->GetLibrary().GetOpCond();
            Volt log0_v = op_cond.GetSupply()->GetGndVoltage();
            Volt log1_v = op_cond.GetSupply()->GetVddVoltage();

            sim->SetDuration(2);

            // Tie async pins to inactive value
            for (auto & a_pin : cell.GetPins(PinKind::ASYNC)) {
                sim->AddStimuli(&a_pin, Stimulus(((a_pin.GetPolarity()) == 1) ? log0_v : log1_v));
            }

            // Drive input data pins
            i = 0;
            for (auto & i_pin : cell.GetPins(PinDirection::IN, PinKind::DATA)) {
                int i_pin_val = (i_pin_vect >> i) & 0x1;
                sim->AddStimuli(&i_pin, (i_pin_val == 1) ? log1_v : log0_v);
                i++;
            }

            // Generate clock edge
            for (auto & c_pin : cell.GetPins(PinKind::CLK)) {
                sim->AddStimuli(&c_pin, Stimulus((clock_polarity == 0) ? log0_v : log1_v,
                                                 (clock_polarity == 0) ? log1_v : log0_v,
                                                1.0, 0.01, 0.01, 2, 2, 1));
            }

            sim->PutMetaData(clock_polarity);

            cell.AddSimulation(sim);
            ctx_->GetSimulationPool().EnqueueSimulation(sim);
        }
    }
}

bool Algorithms::MeasureFFClockPolarity(Cell &cell)
{
    assert(cell.GetSequential().GetKind() == SequentialKind::FLIP_FLOP);

    bool out_change_posedge = false;
    bool out_change_negedge = false;

    for (Simulation *sim : cell.GetSimulations()) {

        // TODO: Find better way of filtering sims from previous ones ?
        if (!sim->name_.starts_with("FF_CKPOL")) {
            continue;
        }

        if (!sim->CheckSucesfull()) {
            return false;
        }

        Waves w = sim->ReadWaves();

        for (auto & o_pin : cell.GetPins(PinDirection::OUT)) {
            auto &out_v = w.GetVoltage(o_pin.name_);

            int out_start = ToLogic(out_v.front());
            int out_end = ToLogic(out_v.back());

            if (out_start != out_end) {
                int clock_polarity = sim->GetMetaDataAt(0);
                if (clock_polarity == 0) {
                    out_change_posedge = true;
                } else {
                    out_change_negedge = true;
                }
            }
        }
    }

    if (out_change_negedge && out_change_posedge) {
        Pin &c_pin = cell.GetPins(PinKind::CLK).front();
        error("%s - Flip-flop clock pin %s is active on both rising and falling edge. "
              "Both polarity flip-flops are not supported", c_pin.name_);
        return false;
    }

    if (!(out_change_negedge || out_change_posedge)) {
        Pin &c_pin = cell.GetPins(PinKind::CLK).front();
        error("%s - Flip-flop clock pin %s is not active on rising nor falling edge. ",
              c_pin.name_);
        return false;
    }

    if (out_change_posedge) {
        cell.GetSequential().SetClockPolarity(EdgeKind::RISING);
    } else {
        cell.GetSequential().SetClockPolarity(EdgeKind::FALLING);
    }

    return true;
}

void Algorithms::PrepareFFClockDelaySims(Cell &cell)
{
    for (auto & o_pin : cell.GetPins(PinDirection::OUT)) {

        auto *templ = cell.GetDelayTemplate();
        assert(templ != nullptr);

        // TODO: This is hard-coded for Q with the same polarity!
        // We should extract logic function of each Q and compute according to it!
        // Also, this is missing values of async pins.
        o_pin.AddArc(Arc(&o_pin, templ, ArcKind::SEQ_CK, 0, 1, 0, 1));
        size_t arc_index = o_pin.GetArcs().size() - 1;

        size_t i_tran_index = 0;
        for (NanoSecond i_tran : templ->GetIndex1()) {

            size_t o_cap_index = 0;
            for (PicoFarad o_cap : templ->GetIndex2()) {

                // TODO: This assumes D-style flip-flop.
                for (int d_val = 0; d_val < 2; d_val++) {

                    auto & c_pin = cell.GetPins(PinKind::CLK).front();
                    auto & d_pin = cell.GetPins(PinDirection::IN, PinKind::DATA).front();
                    std::string sim_name = sprintf("SEQ_DLY_%s_%s%d_TRAN_%f_CAP_%f",
                                                    c_pin.name_, d_pin.name_, d_val, i_tran, o_cap);
                    Simulation *sim = NewSimulation(sim_name, SimulationKind::TRAN, &cell);

                    OpCond & op_cond = ctx_->GetLibrary().GetOpCond();
                    Volt log0_v = op_cond.GetSupply()->GetGndVoltage();
                    Volt log1_v = op_cond.GetSupply()->GetVddVoltage();

                    sim->SetDuration(25);

                    // Tie async pins to inactive value
                    for (auto & a_pin : cell.GetPins(PinKind::ASYNC)) {
                        sim->AddStimuli(&a_pin, Stimulus(((a_pin.GetPolarity()) == 1) ? log0_v : log1_v));
                    }

                    // Drive D-pin
                    // First preset to !d_val and then one transfer to d_val.
                    sim->AddStimuli(&d_pin, Stimulus((d_val == 0) ? log0_v : log1_v,
                                                     (d_val == 0) ? log1_v : log0_v,
                                                     0.1, 0.01, 0.01, 10.0, 100.0, 1));

                    // Drive clock
                    EdgeKind c_polarity = cell.GetSequential().GetClockPolarity();
                    sim->AddStimuli(&c_pin, Stimulus(
                                        (c_polarity == EdgeKind::RISING) ? log0_v : log1_v,
                                        (c_polarity == EdgeKind::RISING) ? log1_v : log0_v,
                                         5.0, i_tran, i_tran, 5.0, 10.0, 2));

                    // Add load
                    sim->AddLoad(&o_pin, o_cap);
                    sim->PutMetaData(d_val);

                    o_pin.GetArcs()[arc_index].AddSimulation(i_tran_index, o_cap_index, sim);
                    ctx_->GetSimulationPool().EnqueueSimulation(sim);
                }
                o_cap_index++;
            }
            i_tran_index++;
        }
    }
}

bool Algorithms::MeasureFFClockDelay(Cell &cell)
{
    for (auto & o_pin : cell.GetPins(PinDirection::OUT)) {
        for (auto & arc : o_pin.GetArcs()) {

            size_t i_tran_index = 0;
            for (auto & sims_row : arc.GetSimulations()) {

                size_t o_cap_index = 0;
                for (auto & sims_row_coll : sims_row) {

                    assert(sims_row_coll.size() == 2);
                    for (Simulation *sim : sims_row_coll) {

                        // TODO: Find better way of filtering sims
                        if (!sim->name_.starts_with("SEQ_DLY")) {
                            continue;
                        }

                        if (!sim->CheckSucesfull()) {
                            return false;
                        }

                        Waves w = sim->ReadWaves();

                        int d_val = sim->GetMetaDataAt(0);
                        auto & c_pin = cell.GetPins(PinKind::CLK).front();

                        Variables &vars = ctx_->GetVariables();
                        double q_th = (d_val == 1) ? vars.GetDoubleVariable("delay_out_rise") :
                                                    vars.GetDoubleVariable("delay_out_fall");
                        double c_th = (cell.GetSequential().GetClockPolarity() == EdgeKind::RISING) ?
                                                vars.GetDoubleVariable("delay_in_rise") :
                                                vars.GetDoubleVariable("delay_in_fall");

                        Supply *supply = ctx_->GetLibrary().GetOpCond().GetSupply();
                        Volt vdd_voltage = supply->GetVddVoltage();
                        q_th *= vdd_voltage;
                        c_th *= vdd_voltage;

                        // TODO: Pass the expected times via Simulation Object
                        NanoSecond q_edge = w.FindTransitionTime(o_pin.name_, q_th, 10.0, 25.0);
                        NanoSecond c_edge = w.FindTransitionTime(c_pin.name_, c_th, 12.5, 17.5);

                        NanoSecond ck_to_q = q_edge - c_edge;
                        if (d_val == 1) {
                            arc.SetRiseDelay(i_tran_index, o_cap_index, ck_to_q);
                        } else {
                            arc.SetFallDelay(i_tran_index, o_cap_index, ck_to_q);
                        }
                    }

                    o_cap_index++;
                }
                i_tran_index++;
            }
        }
    }

    return true;
}

bool Algorithms::MeasureFFClockTransition(Cell &cell)
{
    for (auto & o_pin : cell.GetPins(PinDirection::OUT)) {
        for (auto & arc : o_pin.GetArcs()) {

            size_t i_tran_index = 0;
            for (auto & sims_row : arc.GetSimulations()) {

                size_t o_cap_index = 0;
                for (auto & sims_row_coll : sims_row) {

                    assert(sims_row_coll.size() == 2);
                    for (Simulation *sim : sims_row_coll) {

                        // TODO: Find better way of filtering sims
                        if (!sim->name_.starts_with("SEQ_DLY")) {
                            continue;
                        }

                        if (!sim->CheckSucesfull()) {
                            return false;
                        }

                        Waves w = sim->ReadWaves();

                        int d_val = sim->GetMetaDataAt(0);

                        Variables &vars = ctx_->GetVariables();
                        double hi_th = (d_val == 1) ? vars.GetDoubleVariable("slew_upper_rise") :
                                                    vars.GetDoubleVariable("slew_upper_fall");
                        double lo_th = (d_val == 1) ? vars.GetDoubleVariable("slew_lower_rise") :
                                                    vars.GetDoubleVariable("slew_lower_fall");
                        Supply *supply = ctx_->GetLibrary().GetOpCond().GetSupply();
                        Volt vdd_voltage = supply->GetVddVoltage();
                        hi_th *= vdd_voltage;
                        lo_th *= vdd_voltage;

                        // TODO: Pass the expected times via Simulation Object
                        NanoSecond hi_time = w.FindTransitionTime(o_pin.name_, hi_th, 12.5, 17.5);
                        NanoSecond lo_time = w.FindTransitionTime(o_pin.name_, lo_th, 12.5, 17.5);

                        if (d_val == 1) {
                            arc.SetRiseTransition(i_tran_index, o_cap_index, hi_time - lo_time);
                        } else {
                            arc.SetFallTransition(i_tran_index, o_cap_index, lo_time - hi_time);
                        }
                    }
                    o_cap_index++;
                }
                i_tran_index++;
            }
        }
    }

    return true;
}

bool Algorithms::MeasureFFClockPowers(Cell &cell)
{
    for (auto & o_pin : cell.GetPins(PinDirection::OUT)) {
        for (auto & arc : o_pin.GetArcs()) {

            size_t i_tran_index = 0;
            for (auto & sims_row : arc.GetSimulations()) {

                size_t o_cap_index = 0;
                for (auto & sims_row_coll : sims_row) {

                    assert(sims_row_coll.size() == 2);
                    for (Simulation *sim : sims_row_coll) {

                        // TODO: Find better way of filtering sims
                        if (!sim->name_.starts_with("SEQ_DLY")) {
                            continue;
                        }

                        if (!sim->CheckSucesfull()) {
                            return false;
                        }

                        Waves w = sim->ReadWaves();

                        int d_val = sim->GetMetaDataAt(0);

                        Supply *supply = ctx_->GetLibrary().GetOpCond().GetSupply();
                        Volt vdd_voltage = supply->GetVddVoltage();
                        Pin *c_pin = cell.GetSequential().GetClockPin();
                        EdgeKind edge_pol = cell.GetSequential().GetClockPolarity();

                        double ck_th = vdd_voltage;
                        ck_th *= (edge_pol == EdgeKind::RISING) ? 0.01 : 0.99;

                        double q_th = vdd_voltage;
                        q_th *= (d_val == 1) ? 0.99 : 0.01;

                        // TODO: Pass the look-up times via the Simulation object
                        size_t start_index = w.FindTransitionIndex(c_pin->name_, ck_th, 12.5, 17.5);
                        size_t end_index = w.FindTransitionIndex(o_pin.name_, q_th, 12.5, 17.5);

                        // Integrate
                        PicoJoule e = 0;
                        auto & i_vdd = w.GetCurrent(supply->GetVddName());
                        auto & i_q = w.GetCurrent(o_pin.name_);
                        NanoSecond time_step = sim->GetTimeStep();

                        NanoWatt lkg = i_vdd[0] * vdd_voltage;

                        // TODO: Integrate with possibly varying time step ?
                        // The power is drain by the cell when i_vdd is negative.
                        // The power is drain by the load when i_out is positive.
                        // Sum of these therefore gives the current consumed by
                        // the cell.
                        // Also, if i_vdd is positive, it means current flows from
                        // VDD back to the supply network. We are conservative and
                        // ignore such flow as it would give more optimistic results.
                        for (size_t i = start_index; i < end_index; i++) {
                            MicroAmp i_vdd_cap = (i_vdd[i] < 0) ? i_vdd[i] : 0.0;
                            e += (((i_vdd_cap + i_q[i]) * vdd_voltage) - lkg) * time_step;
                        }

                        // uA * V = uW ;  uW * ns = femtoJ -> Convert to Pico
                        e /= 1E3;
                        e = std::fabs(e);

                        if (d_val == 1) {
                            arc.SetRisePower(i_tran_index, o_cap_index, e);
                        } else {
                            arc.SetFallPower(i_tran_index, o_cap_index, e);
                        }
                    }
                    o_cap_index++;
                }
                i_tran_index++;
            }
        }
    }

    return true;
}

void Algorithms::PrepareSetupSims(Cell &cell)
{
    auto *templ = cell.GetConstraintTemplate();
    assert(templ != nullptr);

    assert(cell.GetKind() == CellKind::SEQUENTIAL);

    // TODO: This assumes DFF with single output
    auto & d_pin = cell.GetPins(PinDirection::IN, PinKind::DATA).front();
    d_pin.AddArc(Arc(&d_pin, templ, ArcKind::SEQ_SETUP, 0, 1, 0, 1));
    size_t arc_index = d_pin.GetArcs().size() - 1;

    const NanoSecond ck_q_base = std::numeric_limits<NanoSecond>::quiet_NaN();
    const NanoSecond ck_d_skew = 1.0;
    const NanoSecond step = ck_d_skew * 2.0;

    size_t d_tran_index = 0;
    for (NanoSecond d_tran : templ->GetIndex1()) {

        size_t ck_tran_index = 0;
        for (PicoFarad ck_tran : templ->GetIndex2()) {
            PrepareOneSetupSim(cell, arc_index, d_tran_index, d_tran, ck_tran_index,
                               ck_tran, ck_q_base, ck_d_skew, step);
            ck_tran_index++;
        }
        d_tran_index++;
    }
}

void Algorithms::PrepareOneSetupSim(Cell &cell, size_t arc_index,
                                    size_t d_tran_index, NanoSecond d_tran,
                                    size_t ck_tran_index, NanoSecond ck_tran,
                                    NanoSecond ck_q_base, NanoSecond ck_d_skew,
                                    NanoSecond step)
{
    // TODO: This assumes DFF with single output
    auto & c_pin = cell.GetPins(PinKind::CLK).front();
    auto & d_pin = cell.GetPins(PinDirection::IN, PinKind::DATA).front();

    std::string sim_name = sprintf("SEQ_SETUP_%s_TRAN_%f_%s_TRAN_%f_CK_D_SKEW_%f",
                                    d_pin.name_, d_tran, c_pin.name_, ck_tran, ck_d_skew);
    Simulation *sim = NewSimulation(sim_name, SimulationKind::TRAN, &cell);

    OpCond & op_cond = ctx_->GetLibrary().GetOpCond();
    Volt log0_v = op_cond.GetSupply()->GetGndVoltage();
    Volt log1_v = op_cond.GetSupply()->GetVddVoltage();

    sim->SetDuration(25);

    // Tie async pins to inactive value
    for (auto & a_pin : cell.GetPins(PinKind::ASYNC)) {
        sim->AddStimuli(&a_pin, Stimulus(((a_pin.GetPolarity()) == 1) ? log0_v : log1_v));
    }

    NanoSecond t_ck_offset = 5.0;
    NanoSecond t_ck_period = 10.0;
    NanoSecond t_d = t_ck_offset + t_ck_period - ck_d_skew;

    // Drive D-pin
    // First preset Q to 0 on first clock edge, then apply D=1 at ck_d_skew
    // before second clock edge
    sim->AddStimuli(&d_pin, Stimulus(log0_v, log1_v, t_d, d_tran, d_tran, 10.0, 100.0, 1));

    // Drive clock
    EdgeKind c_polarity = cell.GetSequential().GetClockPolarity();
    Volt from = (c_polarity == EdgeKind::RISING) ? log0_v : log1_v;
    Volt to = (c_polarity == EdgeKind::RISING) ? log1_v : log0_v;
    sim->AddStimuli(&c_pin, Stimulus(from, to, t_ck_offset, ck_tran, ck_tran, 5.0, t_ck_period, 2));

    // TODO: Add float type support to metadata
    sim->PutDoubleMetaData(ck_q_base);
    sim->PutDoubleMetaData(ck_d_skew);
    sim->PutDoubleMetaData(step);

    info("PrepareOneSetupSim:");
    info("  ck_q_base:  %f ns", ck_q_base);
    info("  ck_d_skew:  %f ns", ck_d_skew);
    info("  step:       %f ns", step);

    d_pin.GetArcs()[arc_index].AddSimulation(d_tran_index, ck_tran_index, sim);
    ctx_->GetSimulationPool().EnqueueSimulation(sim);
}

std::pair<bool,bool> Algorithms::MeasureSetup(Cell &cell)
{
    assert(cell.GetKind() == CellKind::SEQUENTIAL);

    // TODO: This assumes DFF with single D and Q
    auto & q_pin = cell.GetPins(PinDirection::OUT, PinKind::DATA).front();
    auto & d_pin = cell.GetPins(PinDirection::IN, PinKind::DATA).front();
    auto & c_pin = cell.GetPins(PinKind::CLK).front();

    // Search for arc of this kind
    // TODO: Move the search down to the Pin itself
    int arc_index = -1;
    for (int i = 0; auto & a : d_pin.GetArcs()) {
        if (a.GetKind() == ArcKind::SEQ_SETUP) {
            arc_index = i;
            break;
        }
        i++;
    }

    assert (arc_index != -1);

    auto *templ = cell.GetConstraintTemplate();
    assert(templ != nullptr);

    bool all_finished = true;
    bool all_ok = true;

    size_t d_tran_index = 0;
    for (NanoSecond d_tran : templ->GetIndex1()) {

        size_t ck_tran_index = 0;
        for (PicoFarad ck_tran : templ->GetIndex2()) {

            auto & arc = d_pin.GetArcs()[arc_index];
            auto & sims = arc.GetSimulations()[d_tran_index][ck_tran_index];
            Simulation *sim = sims.back();

            if (!sim->IsFinished()) {
                all_finished = false;
                continue;
            }

            if (!sim->CheckSucesfull()) {
                all_ok = false;
                continue;
            }

            NanoSecond ck_q_base = sim->GetDoubleMetaDataAt(0);
            NanoSecond ck_d_skew = sim->GetDoubleMetaDataAt(1);
            NanoSecond step = sim->GetDoubleMetaDataAt(2);

            info("MeasureSetup:");
            info("  ck_q_base:  %f ns", ck_q_base);
            info("  ck_d_skew:  %f ns", ck_d_skew);
            info("  step:       %f ns", step);

            EdgeKind ck_pol = cell.GetSequential().GetClockPolarity();
            Variables &vars = ctx_->GetVariables();

            double ck_th = (ck_pol == EdgeKind::RISING) ?
                                vars.GetDoubleVariable("delay_in_rise") :
                                vars.GetDoubleVariable("delay_in_fall");
            double q_th = vars.GetDoubleVariable("delay_in_rise");

            Supply *supply = ctx_->GetLibrary().GetOpCond().GetSupply();
            Volt vdd_voltage = supply->GetVddVoltage();
            ck_th *= vdd_voltage;
            q_th *= vdd_voltage;

            Waves w = sim->ReadWaves();
            int q_end = ToLogic(w.GetVoltage(q_pin.name_).back());

            // First simulation just measures the CK -> Q base delay.
            // Proceed further with the same step and skew
            bool is_first = std::isnan(ck_q_base);
            if (is_first) {
                info("  First\n");
                assert (q_end == 1 && "D not captured by clock -> First Setup/Hold Simulation failed");

                // TODO: Pass expected transition times via Simulation object
                NanoSecond ck_edge = w.FindTransitionTime(c_pin.name_, ck_th, 12.5, 17.5);
                NanoSecond q_edge = w.FindTransitionTime(q_pin.name_, q_th, 12.5, 17.5);
                ck_q_base = q_edge - ck_edge;

                PrepareOneSetupSim(cell, arc_index, d_tran_index, d_tran,
                                   ck_tran_index, ck_tran, ck_q_base, ck_d_skew,
                                   step);
                all_finished = false;

            // Q == 1 -> CK edge took the D -> Mesure CK -> Q delay
            } else if (q_end == 1) {
                info("  Q == 1");
                NanoSecond ck_edge = w.FindTransitionTime(c_pin.name_, ck_th, 12.5, 17.5);
                NanoSecond q_edge = w.FindTransitionTime(q_pin.name_, q_th, 12.5, 17.5);
                NanoSecond ck_q_delay = q_edge - ck_edge;

                info("  ck_q_delay: %f", ck_q_delay);

                // TODO: This branch will repeat mutliple times -> Not needed!
                // TODO: Make the percentage range configurable by user!
                info("  Delay increment: %f %", (ck_q_delay - ck_q_base) / (ck_q_base) * 100.0);

                // Upon 2.5 - 5 % delay increment, consider this as S/H time
                if (ck_q_delay > ck_q_base * 1.025) {

                    if (ck_q_delay < ck_q_base * 1.05) {
                        info("  DONE!\n");
                        if (ck_pol == EdgeKind::RISING) {
                            arc.SetRiseConstraint(d_tran_index, ck_tran_index, ck_d_skew);
                        } else {
                            arc.SetFallConstraint(d_tran_index, ck_tran_index, ck_d_skew);
                        }

                    // Q=1, but the delay is above 5 %, iterate down as-if we missed!
                    } else {
                        info("  Delay more than 5 % increment...\n");
                        ck_d_skew += step;
                        step *= 2.0/3.0;
                        all_finished = false;
                        PrepareOneSetupSim(cell, arc_index,
                                           d_tran_index, d_tran,
                                           ck_tran_index, ck_tran,
                                           ck_q_base, ck_d_skew,
                                           step);
                    }

                // Q=1, but the CK->Q delay is less than 2.5 % increment.
                // Need to decrement the skew for next simulation.
                } else {
                    info("  Delay less than 2.5 % increment...\n");
                    ck_d_skew -= step;
                    step *= 2.0/3.0;
                    all_finished = false;
                    PrepareOneSetupSim(cell, arc_index,
                                       d_tran_index, d_tran,
                                       ck_tran_index, ck_tran,
                                       ck_q_base, ck_d_skew,
                                       step);
                }

            // Q == 0 -> Missed by CK edge -> Skew too low (or negative) ->
            // Need to increase the skew.
            } else if (q_end == 0) {
                info("  Q == 0\n");
                ck_d_skew += step;
                step *= 2.0/3.0;
                all_finished = false;
                PrepareOneSetupSim(cell, arc_index,
                                   d_tran_index, d_tran,
                                   ck_tran_index, ck_tran,
                                   ck_q_base, ck_d_skew,
                                   step);
            }
            ck_tran_index++;
        }
        d_tran_index++;
    }

    return std::pair<bool,bool>(all_finished, all_ok);
}

#define PROCESS_RESULTS(cell, func)                                             \
    {                                                                           \
        bool ok = func(cell);                                                   \
        if (!ok) {                                                              \
            error("%s - Aborting characterization !", cell.GetName());          \
            cell.SetCharactState(CharactState::ERROR);                          \
            break;                                                              \
        }                                                                       \
    }

bool Algorithms::CharacterizeLibrary()
{
    SimulationPool &sp = ctx_->GetSimulationPool();

    sp.SetNumThreads(ctx_->GetVariables().GetIntVariable("max_threads"));
    sp.Start();

    bool finished = false;
    do {
        for (auto & [cell_name, cell] : ctx_->GetLibrary().GetCells()) {

            // Per-cell simulations jobs state machine
            switch (cell.GetCharactState()) {

            case CharactState::START:
                info("%s - Launching sanity simulation", cell.GetName());
                PrepareSanitySim(cell);
                cell.SetCharactState(CharactState::SANITY);
                break;

            case CharactState::SANITY:
            {
                if (!cell.IsSimulationFinished()) {
                    continue;
                }

                PROCESS_RESULTS(cell, CheckSanitySim);

                info("%s - Launching input capacitance simulations", cell.GetName());
                PrepareInputCapSims(cell);

                cell.SetCharactState(CharactState::INPUT_CAP);
                break;
            }

            case CharactState::INPUT_CAP:
            {
                if (!cell.IsSimulationFinished()) {
                    continue;
                }

                info("%s - Measuring input capacitance", cell.GetName());
                PROCESS_RESULTS(cell, MeasureInputCap);

                if (cell.GetKind() == CellKind::COMBINATIONAL) {
                    info("%s - Launching combinatorial logic table simulations", cell.GetName());
                    PrepareComboLogicTableAndLeakageSims(cell);
                    cell.SetCharactState(CharactState::COM_LOG_TBL_LKG);
                } else {
                    info("%s - Launching asynchronous pins simulations", cell.GetName());
                    PrepareSeqAsyncFunctionSims(cell);
                    cell.SetCharactState(CharactState::SEQ_ASYNC_FUNCS);
                }
                break;
            }

            case CharactState::COM_LOG_TBL_LKG:
            {
                if (!cell.IsSimulationFinished()) {
                    continue;
                }

                info("%s - Measuring logic table", cell.GetName());
                PROCESS_RESULTS(cell, MeasureComboLogicTables);

                info("%s - Measuring leakage", cell.GetName());
                PROCESS_RESULTS(cell, MeasureComboLeakage);

                info("%s - Calculating logic function", cell.GetName());
                CalculateComboLogicFunctions(cell);

                info("%s - Launching delay and power simulations", cell.GetName());
                PrepareComboDelayAndPowerSims(cell);

                cell.SetCharactState(CharactState::COM_DLY_PWR);
                break;
            }

            case CharactState::COM_DLY_PWR:
                if (!cell.IsSimulationFinished()) {
                    continue;
                }

                info("%s - Measuring output delays", cell.GetName());
                PROCESS_RESULTS(cell, MeasureComboDelays);

                info("%s - Measuring output transitions", cell.GetName());
                PROCESS_RESULTS(cell, MeasureComboTransitions);

                info("%s - Measuring internal power", cell.GetName());
                PROCESS_RESULTS(cell, MeasureComboPowers);

                cell.SetCharactState(CharactState::DONE);
                break;

            case CharactState::SEQ_ASYNC_FUNCS:
                if (!cell.IsSimulationFinished()) {
                    continue;
                }

                info("%s - Measuring async pin functions", cell.GetName());
                PROCESS_RESULTS(cell, MeasureSeqAsyncFunctions);

                info("%s - Launching sequential cell kind detection simulations", cell.GetName());
                PrepareSeqCellKindSims(cell);

                cell.SetCharactState(CharactState::SEQ_CELL_KIND);
                break;

            case CharactState::SEQ_CELL_KIND:
                if (!cell.IsSimulationFinished()) {
                    continue;
                }

                info("%s - Measuring if cell is edge or level sensitive", cell.GetName());
                PROCESS_RESULTS(cell, MeasureSeqCellKind);

                if (cell.GetSequential().GetKind() == SequentialKind::FLIP_FLOP) {
                    info("%s - Preparing flip-flop clock polarity detection simulations",
                         cell.GetName());
                    PrepareFFClockPolaritySims(cell);
                    cell.SetCharactState(CharactState::SEQ_FF_CK_POL);
                } else {
                    cell.SetCharactState(CharactState::DONE);
                }
                break;

            case CharactState::SEQ_FF_CK_POL:
                if (!cell.IsSimulationFinished()) {
                    continue;
                }

                info("%s - Measuring clock polarity", cell.GetName());
                PROCESS_RESULTS(cell, MeasureFFClockPolarity);

                info("%s - Launching clock to output delay simulations", cell.GetName());
                PrepareFFClockDelaySims(cell);

                cell.SetCharactState(CharactState::SEQ_FF_DLY_PWR);
                break;

            case CharactState::SEQ_FF_DLY_PWR:
                if (!cell.IsSimulationFinished()) {
                    continue;
                }

                info("%s - Measuring clock to output delay", cell.GetName());
                PROCESS_RESULTS(cell, MeasureFFClockDelay);
                info("%s - Measuring output transition", cell.GetName());
                PROCESS_RESULTS(cell, MeasureFFClockTransition);
                info("%s - Measuring internal power", cell.GetName());
                PROCESS_RESULTS(cell, MeasureFFClockPowers);

                cell.SetCharactState(CharactState::SEQ_FF_SETUP_START);
                break;

            case CharactState::SEQ_FF_SETUP_START:
                info("%s - Preparing flip-flop Setup Simulations", cell.GetName());
                PrepareSetupSims(cell);
                cell.SetCharactState(CharactState::SEQ_FF_SETUP_FINISH);
                break;

            case CharactState::SEQ_FF_SETUP_FINISH:
            {
                if (!cell.IsSimulationFinished()) {
                    continue;
                }

                auto [all_setups_found, sim_ok] = MeasureSetup(cell);
                if (!sim_ok) {
                    error("%s - Aborting characterization !", cell.GetName());
                    cell.SetCharactState(CharactState::ERROR);
                }

                if (!all_setups_found) {
                    continue;
                }

                cell.SetCharactState(CharactState::DONE);
                break;
            }

            default:
                break;
            }
        }

        finished = true;
        for (auto & [cell_name, cell] : ctx_->GetLibrary().GetCells()) {
            CharactState state = cell.GetCharactState();
            if (state != CharactState::DONE && state != CharactState::ERROR) {
                finished = false;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    } while (!finished);

    sp.WaitDone();

    size_t ok_cells = 0;
    size_t err_cells = 0;

    for (auto & [cell_name, cell] : ctx_->GetLibrary().GetCells()) {
        CharactState state = cell.GetCharactState();
        if (state == CharactState::DONE) {
            ok_cells++;
        }
        if (state == CharactState::ERROR) {
            err_cells++;
        }
    }

    info(std::string(80, '*'));
    if (err_cells == 0) {
        info("Characterization SUCEEDED:");
    } else {
        info("Characterization FAILED:");
    }
    info(std::string(80, '*'));
    info("Sucessfull cells:  %d", ok_cells);
    info("Error cells:       %d", err_cells);
    info(std::string(80, '*'));

    return (err_cells == 0) ? true : false;
}

}
