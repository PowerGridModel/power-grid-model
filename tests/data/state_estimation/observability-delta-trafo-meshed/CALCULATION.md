<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# Simplified Dyn5 with an LV mesh: manual calculation and output reference

This note reproduces [asym_output.json](asym_output.json) with hand calculations from [input.json](input.json)
for a meshed network that is based on [issue #1668](https://github.com/PowerGridModel/power-grid-model/issues/1668).

[asym_output.json](asym_output.json) contains the expected result after **assuming zero zero-sequence voltage at the Delta side**, $V_{M0}=0$.

The derivation uses node 2 LV phase a as its angular reference, $\theta_a = \arg V_a=0$. Section 5 converts the results to PGM's reference before storing them in `asym_output.json`, as PGM chooses the slack bus (bus 1) as the angle reference.

## 1. Symbols and inputs

### 1.1 Symbols

| Index or label | Allowed values and meaning | Applies to |
|---|---|---|
| $p\in\{a,b,c\}$ | LV phase index. | $V_p,U_p,\theta_p,I_p,S_p,P_p,Q_p,E_p$ |
| $q\in\{A,B,C\}$ | MV phase index. $(a,b,c)$ correspond to $(A,B,C)$. | $V_q,I_{Mq},S_{Mq}$ |
| $k\in\{0,1,2\}$ | Sequence index: 0 = zero, 1 = positive, 2 = negative. | $V_{Mk},V_{Lk},I_{Mk},I_{Lk}$ |
| $k\in\{1,2\}$ | Only the two sequences that transfer through the Dyn transformer. | $t_k,E_k$ and the [original sequence circuits](../observability-delta-trafo/CALCULATION.md#22-positive--and-negative-sequence-circuits). No $t_0$ is defined. |
| Side labels $\{M,L\}$ | $M$ = MV, $L$ = LV. | Terminal sequence quantities and rated voltages $U_M,U_L$ |
| Ordered phase pairs $\{AB,BC,CA\}$ | Voltage: first phase minus second. Winding current: first phase toward second. | $V_{AB},V_{BC},V_{CA}$, $I_{AB},I_{BC},I_{CA}$ |
| Terminal labels `from`, `to` | `from` = MV, `to` = LV for the transformer; line directions follow the input. | Transformer and line output fields |
| Line terminal labels $f,t$ | $f$ = from side, $t$ = to side. Positive current and power enter the line at the named terminal. | Line current and power subscripts |
| Node index $i\in\{1,2,8,9\}$ | Node 1 is MV; nodes 2, 8, and 9 are LV. | $V_{i,p},S_{i,p}$ |
| Line index $\ell\in\{10,11,12\}$ | Line ID. Sensor 17 measures line 10 at its from side. | $I_{\ell,p,f},I_{\ell,p,t},S_{\ell,p,f},S_{\ell,p,t}$ |



| Symbol | Meaning | Unit |
|---|---|---|
| $V_p,U_p,\theta_p$, $V_q$ | LV phase-to-ground voltage, magnitude, and angle: $V_p=U_pe^{j\theta_p}$. The final symbol denotes MV phase-to-ground voltage. | V, V, rad or degree, V |
| $V_{AB},V_{BC},V_{CA}$ | MV line voltages. $V_{AB}=V_A-V_B$, cyclically. | V |
| $I_p,S_p=P_p+jQ_p$, $I_{Mq},S_{Mq}$ | LV and MV phase currents and powers: $S_p=V_pI_p^*$, $S_{Mq}=V_qI_{Mq}^*$. Currents enter the transformer. | A, VA, with W/var for $P_p/Q_p$ |
| $I_{AB},I_{BC},I_{CA}$ | Currents inside the MV delta windings, distinct from MV line currents $I_{MA},I_{MB},I_{MC}$. | A |
| $N,I_N$ | Common grounded LV neutral and its return current, directed from $N$ toward earth. | -, A |
| $V_{Mk},V_{Lk},I_{Mk},I_{Lk}$ | Sequence-$k$ voltage and terminal-current phasors on MV ($M$) and LV ($L$). Currents enter the transformer. | V, A |
| $U_M,U_L,U_{\mathrm{rated}}$ | Rated line-to-line voltages. $U_{\mathrm{rated}}$ is the relevant node rating. | V |
| $S_n,u_k,p_k$ | Rated three-phase power, short-circuit voltage, and copper loss (`sn`, `uk`, `pk`). | VA, p.u., W |
| $i_0,p_0$ | No-load current and loss (`i0`, `p0`). $i_0$ is not sequence current $I_0$. | p.u., W |
| $Z_s=jX_s,Z_n$ | LV-referred leakage impedance and LV neutral-grounding impedance. | ohm |
| $Y_{\mathrm{num}}$ | PGM's numerical MV zero-sequence shunt. | S (siemens) |
| $n,r,t_k$ | Winding turns ratio, line-voltage ratio, and complex sequence tap ($k=1,2$ only). | dimensionless |
| $E_p,E_k$ | Internal LV phase-winding voltage and sequence voltage after the ideal tap. | V |
| $\theta_{\mathrm{ref}}\in\mathbb R$ | Manual MV phase-A angle used for the final common reference rotation. | rad |
| $\mathbf V_i,\mathbf I_L$ | Three-phase voltage at node $i$ and current entering the transformer from node 2. $\mathbf V_2=(V_a,V_b,V_c)^T$, $\mathbf I_L=(I_a,I_b,I_c)^T$. | V, A |
| $Z_1,Z_0,\mathbf Z_\ell$ | Positive/negative-sequence line impedance, zero-sequence line impedance, and phase-domain impedance matrix. | ohm |
| $I_{\ell,p,f},I_{\ell,p,t}$ | Phase-$p$ currents entering line $\ell\in\{10,11,12\}$ from its from and to sides. | A |
| $\mathbf I_{\ell,f},\mathbf I_{\ell,t}$ | Corresponding three-phase current vectors. For example, $\mathbf I_{10,f}=(I_{10,a,f},I_{10,b,f},I_{10,c,f})^T$. | A |
| $S_{\ell,p,f},S_{\ell,p,t}$ | Phase-$p$ complex powers $P+jQ$ entering line $\ell$ at its from and to sides. Their magnitudes are the apparent powers. | VA |
| $\mathbf S_{\ell,f},\mathbf S_{\ell,t}$ | Corresponding three-phase power vectors. For example, $\mathbf S_{10,f}=(S_{10,a,f},S_{10,b,f},S_{10,c,f})^T$. | VA |
| $S^{\mathrm{meas}}_{10,p,f}$ | From side complex power of line 10 measured by sensor 17. | VA |
| $\mathbf S_{\mathrm{load},d}$ | Phase powers consumed by load $d\in\{5,13\}$. | VA |

Voltage/current/power phasors and $E_p,E_k,t_k,Z_s,Y_{\mathrm{num}}$ are complex
scalars. Measured magnitudes $U_p>0$ are real.
Active/reactive powers are signed real numbers. Phase angles $\theta_p$ are taken
in $[-180^\circ,180^\circ]$ (equivalently $[-\pi,\pi]$ rad). 

### 1.2 Inputs

The transformer is Dyn5, with both terminals connected, no tap adjustment, and
$Z_n=0$. Its parameters are $U_M/U_L=10000/400$ V, $S_n=100000$ VA,
$u_k=0.04$. We assume $i_0=p_0=p_k=0$ for the sake of simplified hand calculations. The LV measurements from sensors 6 and 7 are:

| Phase | Measured voltage magnitude $U_p$ (V) | Measured active power $P_p$ (W) | Measured reactive power $Q_p$ (var) |
|---|---:|---:|---:|
| a | 230 | 100 | 10 |
| b | 231 | 200 | 20 |
| c | 232 | 300 | 30 |

The voltage measurements are to-ground magnitudes $U_p=|V_p|$. No voltage angles are measured (`u_angle_measured` is absent).

Positive measured power enters the transformer from LV side.

The added LV network is the loop $2\to8\to9\to2$. Node 8 is a zero-injection node.
Load 5 is connected to node 2, and load 13 is connected to node 9. Neither load
has a power measurement. All three lines have
$Z_1=0.1+j0.2\ \Omega$, $Z_0=0.3+j0.6\ \Omega$, zero shunt capacitance,
and rated current $i_n=100$ A.

Sensor 17 measures the from side power of line 10 at node 2, shown rounded as $P+jQ$ in VA:

| Sensor ID | Measured object and terminal | Phase a | Phase b | Phase c |
|---|---|---:|---:|---:|
| 17 | Line 10, `from` (node 2) | $333.766909521+j34.189624486$ | $333.746853954+j34.181466342$ | $333.751998400+j34.160432923$ |

We treat these measurements as exact for hand calculations, using the values in
[input.json](input.json).

## 2. Equivalent circuits

### 2.1 Transformer equivalent circuits

The transformer parameters, sequence circuits, and winding coupling are the same
as in [Sections 2.1–2.4 of the radial network](../observability-delta-trafo/CALCULATION.md#2-equivalent-circuits).
We use the same current directions and Dyn5 winding polarities. The parameters
needed below are

```math
Z_s=j0.064\ \Omega,\qquad n=25\sqrt3,\qquad r=25.
```

### 2.2 LV loop

```text
                              line 10                 line 11
                       I_10,f -->               I_11,f -->
 MV node 1 -- Dyn5 -- node 2 ----------- node 8 ----------- node 9
                       |  \                                |  \
                       |   +--> load 5                     |   +--> load 13
                       |                                   |
                       +------------- line 12 -------------+
                                from node 9 to node 2
                                I_12,f = -2 I_10,f
```

**Figure 1 - LV loop.** Line currents are positive from the `from` node toward the
`to` node. The transformer current $\mathbf I_L$ still points from node 2 into
the transformer. 

For each line, the negative-sequence impedance equals $Z_1$. Transforming the
sequence impedances to phase coordinates gives

```math
\begin{aligned}
\mathbf Z_\ell
&=Z_1\mathbf I_3+\frac{Z_0-Z_1}{3}\mathbf 1\mathbf 1^T\\
&=\begin{bmatrix}
0.166666667+j0.333333333&0.066666667+j0.133333333&0.066666667+j0.133333333\\
0.066666667+j0.133333333&0.166666667+j0.333333333&0.066666667+j0.133333333\\
0.066666667+j0.133333333&0.066666667+j0.133333333&0.166666667+j0.333333333
\end{bmatrix}\ \Omega.
\end{aligned}
```

Here $\mathbf I_3$ is the identity matrix and $\mathbf 1=(1,1,1)^T$.

For these lines, zero shunt capacitance gives $\mathbf I_{\ell,t}=-\mathbf I_{\ell,f}$.
Node 8 has zero injection, so

```math
\mathbf I_{11,f}=-\mathbf I_{10,t}=\mathbf I_{10,f}.
```

Since the line impedances are identical,

```math
\boxed{
\mathbf V_8=\mathbf V_2-\mathbf Z_\ell\mathbf I_{10,f},\qquad
\mathbf V_9=\mathbf V_2-2\mathbf Z_\ell\mathbf I_{10,f}.
}
```

Thus $\mathbf V_8=(\mathbf V_2+\mathbf V_9)/2$. Line 12 has current

```math
\mathbf I_{12,f}
=\mathbf Z_\ell^{-1}(\mathbf V_9-\mathbf V_2)=-2\mathbf I_{10,f}.
```

KCL at node 9 then gives the load current $\mathbf I_{\mathrm{load},13}=3\mathbf I_{10,f}$.

## 3. Calculate MV side voltage and current phasors

We first calculate the phasors at node 2, then the phasors in the LV loop and on
the MV side.

### 3.1 Determine the LV phasors

The transformer parameters and the readings of sensors 6 and 7 are the same as
in [Section 3.1 of the original calculation](../observability-delta-trafo/CALCULATION.md#31-determine-the-lv-phasors).
Using the same ABC phase order and angle reference $\theta_a=0$ gives the following
node-2 phasors, shown rounded:

| Phase | $V_p$ (V) | $I_p$ (A) | $E_p$ (V) |
|---|---:|---:|---:|
| a | $230+j0$ | $0.434782609-j0.043478261$ | $229.997217391-j0.027826087$ |
| b | $-113.973692629-j200.925352777$ | $-0.502487689-j0.710361438$ | $-114.019155761-j200.893193565$ |
| c | $-116.053861454+j200.886787125$ | $-0.534883227+j1.184372250$ | $-115.978061630+j200.921019652$ |

### 3.2 Determine the LV phasors in the loop

Section 3.1 determines the voltage phasors $V_{2,p}=V_p$ at node 2.
The circuit relations in Section 2.2 leave six independent real unknowns for the
LV loop: the real and imaginary parts of
$I_{10,a,f},I_{10,b,f},I_{10,c,f}$. Sensor 17 supplies the six real measurements needed to determine
them: three active powers and three reactive powers.

Sensor 17 measures the from side power of line 10, denoted by
$S^{\mathrm{meas}}_{10,p,f}$. Using
$S^{\mathrm{meas}}_{10,p,f}=V_{2,p}I_{10,p,f}^*$ gives

```math
\boxed{I_{10,p,f}=\left(\frac{S^{\mathrm{meas}}_{10,p,f}}{V_{2,p}}\right)^*.}
```

For phase a, $V_{2,a}=230+j0$ V, so

```math
I_{10,a,f}=\left(\frac{333.766909521+j34.189624486}{230}\right)^*
\approx1.451160476179-j0.148650541244\ \mathrm{A}.
```

The three phase currents at the from side of line 10 are:

| Phase | $I_{10,p,f}$ (A) |
|---|---:|
| a | $1.451160476179-j0.148650541244$ |
| b | $-0.841556277647-j1.183681273411$ |
| c | $-0.592130435602+j1.319315116024$ |

Using the voltage-drop relations from Section 2.2, we calculate the voltages at
nodes 8 and 9. For phase a, including the coupling between phases,

```math
\begin{aligned}
(\mathbf Z_\ell\mathbf I_{10,f})_a
&=(0.1+j0.2)I_{10,a,f}+\frac{0.2+j0.4}{3}(I_{10,a,f}+I_{10,b,f}+I_{10,c,f})\\
&\approx0.177746633+j0.276829096\ \mathrm{V},\\
V_{8,a}&=230-(\mathbf Z_\ell\mathbf I_{10,f})_a
\approx229.822253367-j0.276829096\ \mathrm{V},\\
V_{9,a}&=230-2(\mathbf Z_\ell\mathbf I_{10,f})_a
\approx229.644506734-j0.553658193\ \mathrm{V}.
\end{aligned}
```

The resulting LV phasors are shown rounded:

| Phase | $V_{8,p}$ (V) | $V_{9,p}$ (V) |
|---|---:|---:|
| a | $229.822253367-j0.276829096$ | $229.644506734-j0.553658193$ |
| b | $-114.129173733-j200.640135449$ | $-114.284654837-j200.354918122$ |
| c | $-115.733685865+j200.871819646$ | $-115.413510276+j200.856852166$ |

The remaining currents follow from Section 2.2. Section 4 uses these phasors to
calculate the line and load powers.

### 3.3 Determine the MV side phasors

For Dyn5's winding polarities as shown in [the original Figure 3](../observability-delta-trafo/CALCULATION.md#fig-3),
$V_{AB}=-nE_a$, $V_{BC}=-nE_b$, and $V_{CA}=-nE_c$.
The MV zero-sequence voltage $V_{M0}=(V_A+V_B+V_C)/3$ is a common offset
that the line voltages cannot determine. For example,

```math
V_A=\frac{V_{AB}-V_{CA}}3+V_{M0}
=\frac{n}{3}(E_c-E_a)+V_{M0}.
```

Thus the general reconstruction is

```math
\begin{aligned}
V_A&=\frac{n}{3}(E_c-E_a)+V_{M0},\\
V_B&=\frac{n}{3}(E_a-E_b)+V_{M0},\\
V_C&=\frac{n}{3}(E_b-E_c)+V_{M0}.
\end{aligned}
```

Here $n$ is the winding turns ratio. The results below use the assumption $V_{M0}=0$.
Apply delta KCL to the winding currents in [the original Figure 3](../observability-delta-trafo/CALCULATION.md#fig-3):

```math
\begin{aligned}
(I_{MA},I_{MB},I_{MC})
&=(I_{AB}-I_{CA},\ I_{BC}-I_{AB},\ I_{CA}-I_{BC})\\
&=\frac1n(I_a-I_c,\ I_b-I_a,\ I_c-I_b).
\end{aligned}
```

These are differences of complex phasors, not of current magnitudes.

The numerical-shunt current is zero because $V_{M0}=0$.

Then we calculate for each phase, for example:

```math
\begin{aligned}
V_A&=\frac{25}{\sqrt3}
[(-115.978061630+j200.921019652)-(229.997217391-j0.027826087)]\\
&\approx-4993.723012+j2900.446755\ \mathrm V,\\
I_{MA}&=\frac{(0.434782609-j0.043478261)-(-0.534883227+j1.184372250)}{43.3012701892}\\
&\approx0.022393473256-j0.028355992918\ \mathrm A.
\end{aligned}
```

| MV phase | Voltage (V), rounded | Current (A), rounded |
|---|---:|---:|
| A | $-4993.723012+j2900.446755$ | $0.022393473256-j0.028355992918$ |
| B | $4965.448641+j2899.241850$ | $-0.021645330351-j0.015401007278$ |
| C | $28.274371-j5799.688604$ | $-0.000748142905+j0.043757000195$ |

The LV loop does not supply a new equation for the common MV offset $V_{M0}$.
The additional measurements determine the LV loop but do not remove the delta-side
unobservability.

## 4. Calculate every output field

Use the unrounded phasors from Section 3. For each phase and terminal,

```math
S=VI^*,\qquad p=\mathrm{Re}S,\quad q=\mathrm{Im}S,\quad s=|S|,\quad i=|I|.
```

Apply these rules to the following voltages, currents, and powers. Section 5
converts the voltage angles before writing [asym_output.json](asym_output.json).

| Output field | Calculation |
|---|---|
| Node `u`, `u_pu`, `u_angle` | $\lvert V\rvert$, $\lvert V\rvert/(U_{\mathrm{rated}}/\sqrt3)$, and $\arg V$ in radians, with the reference conversion in Section 5. |
| Transformer `p_from`, `q_from`, `s_from`, `i_from` | Use MV $V_q,I_{Mq}$. |
| Transformer `p_to`, `q_to`, `s_to`, `i_to` | Use LV $V_p,I_p$. |
| Line `p_from`, `q_from`, `s_from`, `i_from` | Use $(V_{2,p},I_{10,p,f})$ for line 10, $(V_{8,p},I_{10,p,f})$ for line 11, and $(V_{9,p},-2I_{10,p,f})$ for line 12. |
| Line `p_to`, `q_to`, `s_to`, `i_to` | Use the to side voltage and the negative of the from side current. |
| Node 1 `p`, `q`; source `p`, `q`, `s`, `i` | Copy the corresponding transformer MV fields. |
| Node 2 `p`, `q` | $S_{2,p}=S_p+3S_{10,p,f}$. |
| Node 8 `p`, `q` | Zero. |
| Node 9 `p`, `q` | $S_{9,p}=-3V_{9,p}I_{10,p,f}^*$. |
| Load 5 `p`, `q`, `s`, `i` | $S_{\mathrm{load},5,p}=-S_{2,p}$, $I_{\mathrm{load},5,p}=-(I_p+3I_{10,p,f})$. |
| Load 13 `p`, `q`, `s`, `i` | $S_{\mathrm{load},13,p}=-S_{9,p}$, $I_{\mathrm{load},13,p}=3I_{10,p,f}$. |
| Source/load `pf` | Signed active power divided by apparent-power magnitude: `p / s`. |
| Sensor residuals | Measured minus calculated voltage magnitude, active power, or reactive power. |

Positive source power and node injection indicate supply to the network. positive
load power indicates consumption. For example,

```math
S_{2,a}=S_a+3S_{10,a,f}
\approx(100+j10)+3(333.766909521+j34.189624486)
\approx1101.300728563+j112.568873458\ \mathrm{VA}.
```

Transformer loading uses the larger sum of phase apparent-power magnitudes:

```math
\texttt{loading}_3
=\frac{\max(\sum_q|S_{Mq}|,\sum_p|S_p|)}{S_n}
=\frac{\max(615.224927726205,602.9925372672534)}{100000}
=0.00615224927726205.
```

Line loading uses the largest phase-current magnitude at either terminal divided
by the rated current $i_n=100$ A:

```math
\texttt{loading}_{10}=\texttt{loading}_{11}
=\frac{\max_p|I_{10,p,f}|}{100}\approx0.014587541640166,\qquad
\texttt{loading}_{12}=\frac{2\max_p|I_{10,p,f}|}{100}\approx0.029175083280332.
```

The reconstructed state satisfies all three sensors. The measurements are satisfied to floating-point precision, so the reference residuals are set to zero. No `u_angle_residual` is stored because no angle is measured.

## 5. Convert to PGM's angle reference and store the output

Apply the common phasor rotation from
[Section 5 of the original calculation](../observability-delta-trafo/CALCULATION.md#5-convert-to-pgms-angle-reference-and-store-the-output),
using MV node 1 phase A as the reference:

```math
\theta_{\mathrm{ref}}=\arg V_A=2.6153966044896544\ \mathrm{rad},\qquad
\texttt{u\_angle}=\arg\!\left(e^{j(\arg V-\theta_{\mathrm{ref}})}\right).
```

Only `node.u_angle` changes; magnitudes and powers are unchanged. Store all fields
from the unrounded phasors in [asym_output.json](asym_output.json).
The reference assumes $V_{M0}=0$. 
