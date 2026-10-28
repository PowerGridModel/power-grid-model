<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# Simplified Dyn5: manual calculation and output reference

This note reproduces [asym_output.json](asym_output.json) with hand calculations from [input.json](input.json)
for [issue #1668](https://github.com/PowerGridModel/power-grid-model/issues/1668).

[asym_output.json](asym_output.json) contains the expected result after **assuming zero zero-sequence voltage at the Delta side**, $`V_{M0}=0`$.

The derivation uses LV phase a as its angular reference, $`\theta_a = \arg V_a=0`$. Section 5 converts the results to PGM's reference before storing them in `asym_output.json`, as PGM chooses the slack bus (bus 1) as the angle reference.

## 1. Symbols and inputs


| Index or label | Allowed values and meaning | Applies to |
|---|---|---|
| $`p\in\{a,b,c\}`$ | LV phase index. | $`V_p,U_p,\theta_p,I_p,S_p,P_p,Q_p,E_p,h_p,L_p`$ |
| $`q\in\{A,B,C\}`$ | MV phase index. $`(a,b,c)`$ correspond to $`(A,B,C)`$. | $`V_q,I_{Mq},S_{Mq}`$ |
| $`k\in\{0,1,2\}`$ | Sequence index: 0 = zero, 1 = positive, 2 = negative. | $`V_{Mk},V_{Lk},I_{Mk},I_{Lk}`$ |
| $`k\in\{1,2\}`$ | Only the two sequences that transfer through the Dyn transformer. | $`t_k,E_k`$ and Figure 1. No $`t_0`$ is defined. |
| Side labels $`\{M,L\}`$ | $`M`$ = MV, $`L`$ = LV. | Terminal sequence quantities and rated voltages $`U_M,U_L`$ |
| Ordered phase pairs $`\{AB,BC,CA\}`$ | Voltage: first phase minus second. Winding current: first phase toward second. | $`V_{AB},V_{BC},V_{CA}`$, $`I_{AB},I_{BC},I_{CA}`$ |
| Terminal labels `from`, `to` | `from` = MV, `to` = LV in this input. | Transformer output fields |



| Symbol | Meaning | Unit |
|---|---|---|
| $`V_p,U_p,\theta_p`$, $`V_q`$ | LV phase-to-ground voltage, magnitude, and angle: $`V_p=U_pe^{j\theta_p}`$. The final symbol denotes MV phase-to-ground voltage. | V, V, rad or degree, V |
| $`V_{AB},V_{BC},V_{CA}`$ | MV line voltages. $`V_{AB}=V_A-V_B`$, cyclically. | V |
| $`I_p,S_p=P_p+jQ_p`$, $`I_{Mq},S_{Mq}`$ | LV and MV phase currents and powers: $`S_p=V_pI_p^*`$, $`S_{Mq}=V_qI_{Mq}^*`$. Currents enter the transformer. | A, VA, with W/var for $`P_p/Q_p`$ |
| $`I_{AB},I_{BC},I_{CA}`$ | Currents inside the MV delta windings, distinct from MV line currents $`I_{MA},I_{MB},I_{MC}`$. | A |
| $`N,I_N`$ | Common grounded LV neutral and its return current, directed from $`N`$ toward earth. | -, A |
| $`V_{Mk},V_{Lk},I_{Mk},I_{Lk}`$ | Sequence-$`k`$ voltage and terminal-current phasors on MV ($`M`$) and LV ($`L`$). Currents enter the transformer. | V, A |
| $`U_M,U_L,U_{\mathrm{rated}}`$ | Rated line-to-line voltages. $`U_{\mathrm{rated}}`$ is the relevant node rating. | V |
| $`S_n,u_k,p_k`$ | Rated three-phase power, short-circuit voltage, and copper loss (`sn`, `uk`, `pk`). | VA, p.u., W |
| $`i_0,p_0`$ | No-load current and loss (`i0`, `p0`). $`i_0`$ is not sequence current $`I_0`$. | p.u., W |
| $`Z_s=jX_s,Z_n`$ | LV-referred leakage impedance and LV neutral-grounding impedance. | ohm |
| $`Y_{\mathrm{num}}`$ | PGM's numerical MV zero-sequence shunt. | S (siemens) |
| $`n,r,t_k`$ | Winding turns ratio, line-voltage ratio, and complex sequence tap ($`k=1,2`$ only). | dimensionless |
| $`E_p,E_k`$ | Internal LV phase-winding voltage and sequence voltage after the ideal tap. | V |
| $`h_p,L_p`$ | Auxiliary coefficient $`h_p=E_pe^{-j\theta_p}`$ and its magnitude $`L_p=\lvert h_p\rvert`$. | V |
| $`z\in\mathbb C`$ | Internal-voltage ratio $`z=E_b/E_a`$. Its real and imaginary parts are $`\mathrm{Re}z,\mathrm{Im}z\in\mathbb R`$. It is not a phase index or an impedance. | dimensionless |
| $`\theta_{\mathrm{ref}}\in\mathbb R`$ | Manual MV phase-A angle used for the final common reference rotation. | rad |

Voltage/current/power phasors and $`E_p,E_k,h_p,t_k,Z_s,Y_{\mathrm{num}}`$ are complex
scalars. Measured magnitudes $`U_p>0`$ and coefficient magnitudes $`L_p\ge0`$ are real.
Active/reactive powers are signed real numbers. Phase angles $`\theta_p`$ are taken
in $`[-180^\circ,180^\circ]`$ (equivalently $`[-\pi,\pi]`$ rad). These continuous
quantities are not discrete indices.

The transformer is Dyn5, with both terminals connected, no tap adjustment, and
$`Z_n=0`$. Its parameters are $`U_M/U_L=10000/400`$ V, $`S_n=100000`$ VA,
$`u_k=0.04`$. We assume $`i_0=p_0=p_k=0`$ to the sake of simplified hand calculations. The LV measurements are:

| Phase | Measured voltage magnitude $`U_p`$ (V) | Measured active power $`P_p`$ (W) | Measured reactive power $`Q_p`$ (var) |
|---|---:|---:|---:|
| a | 230 | 100 | 10 |
| b | 231 | 200 | 20 |
| c | 232 | 300 | 30 |

The voltage measurements are to-ground magnitudes $`U_p=|V_p|`$. No voltage angles are measured (`u_angle_measured` is absent).

Positive measured power enters the transformer from LV side.

We treat these measurements
as exact for hand calculations.

## 2. Equivalent circuits

### 2.1 Simplified transformer parameters

Zero `i0` and `p0` remove the magnetizing branches. Zero `pk` removes series resistance. The LV impedance:

```math
Z_s=j\,u_k\frac{U_L^2}{S_n}=j\,0.04\frac{400^2}{100000}=j0.064\ \Omega,
\qquad n=\frac{10000}{400/\sqrt3}=25\sqrt3,\quad r=25.
```

### 2.2 Positive- and negative-sequence circuits

```text
                       Ideal complex tap t_k
          I_Mk -->                                  <-- I_Lk
 MV (+) o-----------+           +-------o----[ Z_s ]---------o LV (+)
                    |           |       E_k   j0.064 ohm
 V_Mk               )    ||     (                              V_Lk
                    )    ||     (
 MV (-) o-----------+           +----------------------------o LV (-)
 MV sequence reference                              LV sequence reference

 k = 1: t_1 = 25 exp(+j150 deg)       k = 2: t_2 = 25 exp(-j150 deg)
 Winding resistance = 0. Magnetizing branches omitted (i0 = p0 = pk = 0).
```

**Figure 1 - Positive and negative sequence ($`k=1,2`$ only).** Both current arrows
point into the transformer. The leakage impedance is referred to LV.

To calculate the complex tap's phase shift, let $`V_{AB,k}`$ denote the sequence-$`k`$ component of the MV line voltage for
$`k\in\{1,2\}`$. For positive sequence, $`V_{AB,1}`$ leads $`V_{M1}`$ by $`30^\circ`$,
so $`\angle V_{M1}=\angle V_{AB,1}-30^\circ`$. Dyn5 polarity gives
$`E_1=-V_{AB,1}/n`$. Now subtract:

```math
\begin{aligned}
\angle E_1-\angle V_{M1}
&\equiv(\angle V_{AB,1}-180^\circ)
-(\angle V_{AB,1}-30^\circ)\\
&=-180^\circ+30^\circ\\
&=\boxed{-150^\circ}.
\end{aligned}
```

For negative sequence, $`V_{AB,2}`$ lags $`V_{M2}`$ by $`30^\circ`$, giving

```math
\angle E_2-\angle V_{M2}
\equiv(\angle V_{AB,2}+180^\circ)-(\angle V_{AB,2}+30^\circ)
=+150^\circ.
```

Here $`\angle`$ denotes the phasor angle, equivalently $`\arg`$. Angular equivalence
is modulo $`360^\circ`$. Thus $`t_1=25e^{j150^\circ}`$ and
$`t_2=25e^{-j150^\circ}`$, with the same leakage impedance in both circuits:

```math
E_k=V_{Mk}/t_k,\qquad
V_{Lk}-E_k=Z_sI_{Lk},\qquad
I_{Mk}=-I_{Lk}/t_k^*.
```

### 2.3 Zero-sequence circuit

```text
                  MV port                               LV port
                  I_M0 |                                I_L0 |
                       v                                     v
            V_M0 o-----+-------------+                  V_L0 o
                       |             |                       |
                   [ Z_num ]         o                    [ Z_s ]
                  j4e9 ohm           OPEN               j0.064 ohm
                  (PGM only)         o                       |
                       |             |                       |
 Model reference o-----+-------------+                       o
                            Physical delta path       LV neutral / earth
                                                       Z_n = 0 ohm

                  No zero-sequence transfer between the external ports.
```

**Figure 2 - Zero sequence.** The physical MV delta path is open because the delta
connection blocks external zero-sequence current. PGM adds
[a small numerical shunt](https://power-grid-model.readthedocs.io/en/stable/user_manual/calculations.html#floating-grid-handling)
alongside it.

LV zero-sequence current returns through the grounded neutral and induces circulating
current inside the MV delta. These circulating currents cancel at the external MV
terminals, so there is no zero-sequence transfer between the external ports. In this
simplified model, the LV zero-sequence port therefore sees the leakage impedance $`Z_s`$.

The zero-sequence currents are then:

```math
Y_{\mathrm{num}}=-j10^{-8}\frac{S_n}{u_kU_M^2}=-j2.5\times10^{-10}\ \mathrm S,
\qquad I_{M0}=Y_{\mathrm{num}}V_{M0},\quad I_{L0}=V_{L0}/Z_s.
```

### 2.4 Three-phase winding coupling

<a id="fig-3"></a>

```text
          I_AB -->                              <-- I_a
 A o----------+              +----o E_a---[ Z_s ]--------o a
            * )     ||       (
              )     ||       ( *
 B o----------+              +--------------------------o N

          I_BC -->                              <-- I_b
 B o----------+              +----o E_b---[ Z_s ]--------o b
            * )     ||       (
              )     ||       ( *
 C o----------+              +--------------------------o N

          I_CA -->                              <-- I_c
 C o----------+              +----o E_c---[ Z_s ]--------o c
            * )     ||       (
              )     ||       ( *
 A o----------+              +--------------------------o N
```

**Figure 3 - Three coupled winding pairs: AB/aN, BC/bN, and CA/cN.**


| MV/LV winding pair | Voltage relation | Ideal winding-current relation |
|---|---|---|
| AB / aN | $`E_a=-V_{AB}/n`$ | $`I_{AB}=I_a/n`$ |
| BC / bN | $`E_b=-V_{BC}/n`$ | $`I_{BC}=I_b/n`$ |
| CA / cN | $`E_c=-V_{CA}/n`$ | $`I_{CA}=I_c/n`$ |



## 3. Calculate MV side voltage and current phasors

We first calculate the phasors on the LV side, then the phasors on the MV side.

### 3.1 Determine the LV phasors

To reconstruct the MV phasors, first determine the LV quantities for each
$`p\in\{a,b,c\}`$. The voltage magnitude $`U_p`$ and powers $`P_p,Q_p`$ are measured,
but the voltage angle $`\theta_p`$ is unknown. Write the terminal voltage and the
current entering the transformer using $`S_p=P_p+jQ_p=V_pI_p^*`$:

```math
\begin{aligned}
V_p&=U_pe^{j\theta_p},\\
I_p&=\left(\frac{S_p}{V_p}\right)^*
=\frac{P_p-jQ_p}{U_p}e^{j\theta_p}.
\end{aligned}
```

The internal winding voltage $`E_p`$ is the terminal voltage minus the voltage
across the leakage impedance. Substituting these expressions and factoring out
their common rotation gives

```math
\begin{aligned}
E_p&=V_p-Z_sI_p\\
&=U_pe^{j\theta_p}-Z_s\frac{P_p-jQ_p}{U_p}e^{j\theta_p}\\
&=\left[U_p-\frac{Z_s(P_p-jQ_p)}{U_p}\right]e^{j\theta_p}.
\end{aligned}
```

Everything inside the brackets is known from the measurements and transformer
parameters. Denote this complex coefficient by $`h_p`$:

```math
\boxed{h_p=U_p-\frac{Z_s(P_p-jQ_p)}{U_p}
},\qquad E_p=h_pe^{j\theta_p}.
```

The coefficient $`h_p`$ has units of volts and represents the internal winding
voltage relative to its own phase's terminal-voltage angle.  Multiplication by $`e^{j\theta_p}`$ places
it in the common reference without changing its magnitude, so $`|E_p|=|h_p|`$ is
already known. Calculating $`\theta_p`$ then fully determines the LV voltage phasors.

By definition, the LV zero-sequence voltage and current are the averages of the
three phase phasors:

```math
V_{L0}=\frac{V_a+V_b+V_c}{3},\qquad
I_{L0}=\frac{I_a+I_b+I_c}{3}.
```

The same leakage impedance $`Z_s`$ applies to each phase. Adding the three
relations $`E_p=V_p-Z_sI_p`$ therefore gives

```math
\begin{aligned}
E_a+E_b+E_c
&=(V_a-Z_sI_a)+(V_b-Z_sI_b)+(V_c-Z_sI_c)\\
&=(V_a+V_b+V_c)-Z_s(I_a+I_b+I_c)\\
&=3V_{L0}-3Z_sI_{L0}.
\end{aligned}
```

For this transformer, the **LV-side zero-sequence equivalent circuit**
(Figure 2) gives $`V_{L0}=Z_sI_{L0}`$. Substitution makes the last expression zero,
so

```math
\boxed{E_a+E_b+E_c=0}.
```

Substituting $`E_p=h_pe^{j\theta_p}`$ gives the full equation

```math
h_ae^{j\theta_a}+h_be^{j\theta_b}+h_ce^{j\theta_c}=0.
```

With $`\theta_a=0`$, $`E_a=h_a`$, so this becomes

```math
h_a+h_be^{j\theta_b}+h_ce^{j\theta_c}=0.
```

The known coefficients are

```math
(h_a,h_b,h_c)\approx
(229.997217-j0.027826,\ 230.994459-j0.055411,\ 231.991724-j0.082759)\ \mathrm V.
```

Define $`L_p=|h_p|`$ and $`z=E_b/E_a=(h_b/h_a)e^{j\theta_b}`$.
All $`h_p`$ are nonzero for this input. Dividing the equation by $`h_a`$ gives

```math
1+\underbrace{\frac{h_b}{h_a}e^{j\theta_b}}_{z}
+\frac{h_c}{h_a}e^{j\theta_c}=0.
```

This complex equation supplies two real equations for the two real unknowns
$`\theta_b`$ and $`\theta_c`$. To express the internal voltages in terms of $`z`$, first rearrange the equation

```math
E_c=-h_a(1+z).
```


Taking magnitudes of both sides, using $`|E_p|=|h_p|=L_p`$, gives

```math
\begin{aligned}
|E_c|&=|h_a|\,|1+z| &&\Rightarrow\quad |1+z|=\frac{L_c}{L_a}.
\end{aligned}
```

Note that $`|z| = \dfrac{L_b}{L_a}`$, then the identity $`|1+z|^2-|z|^2=1+2\mathrm{Re}z`$ determines the real part
$`\mathrm{Re}z`$. To find the imaginary part:

```math
|z|^2=(\mathrm{Re}z)^2+(\mathrm{Im}z)^2
\quad\Rightarrow\quad
\mathrm{Im}z=\pm\sqrt{\left(\frac{L_b}{L_a}\right)^2-(\mathrm{Re}z)^2}.
```

The magnitude fixes the size of the imaginary part, but not its sign. Since
$`z=E_b/E_a`$, its angle describes angular the position of $`E_b`$ relative to $`E_a`$. Because phase b is approximately $`120^\circ`$ behind phase a, choose the negative root:

```math
\boxed{
\mathrm{Re}z=\frac{L_c^2-L_a^2-L_b^2}{2L_a^2},\qquad
z=\mathrm{Re}z-j\sqrt{\left(\frac{L_b}{L_a}\right)^2-(\mathrm{Re}z)^2}.
}
```

Plug in the numbers:

```math
\mathrm{Re}z\approx-0.495635775218,\qquad
z\approx-0.495635775218-j0.873519199266.
```

Recover the internal voltages $`E_p`$:

```math
\boxed{E_a=h_a,\qquad E_b=h_az,\qquad E_c=-h_a(1+z)},\qquad
V_p=U_p\frac{E_p}{h_p},\quad I_p=\left(\frac{S_p}{V_p}\right)^*.
```

The resulting LV phasors are shown rounded:

| Phase | $`V_p`$ (V) | $`I_p`$ (A) | $`E_p`$ (V) |
|---|---:|---:|---:|
| a | $`230+j0`$ | $`0.434782609-j0.043478261`$ | $`229.997217391-j0.027826087`$ |
| b | $`-113.973692629-j200.925352777`$ | $`-0.502487689-j0.710361438`$ | $`-114.019155761-j200.893193565`$ |
| c | $`-116.053861454+j200.886787125`$ | $`-0.534883227+j1.184372250`$ | $`-115.978061630+j200.921019652`$ |

### 3.2 Determine the MV side phasors

For Dyn5's winding polarities as shown in [Fig. 3](#fig-3),
$`V_{AB}=-nE_a`$, $`V_{BC}=-nE_b`$, and $`V_{CA}=-nE_c`$.
The MV zero-sequence voltage $`V_{M0}=(V_A+V_B+V_C)/3`$ is a common offset
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

Here $`n`$ is the winding turns ratio. The results below use the assumption $`V_{M0}=0`$.
Apply delta KCL to the winding currents in Figure 3:

```math
\begin{aligned}
(I_{MA},I_{MB},I_{MC})
&=(I_{AB}-I_{CA},\ I_{BC}-I_{AB},\ I_{CA}-I_{BC})\\
&=\frac1n(I_a-I_c,\ I_b-I_a,\ I_c-I_b).
\end{aligned}
```

These are differences of complex phasors, not of current magnitudes.

The numerical-shunt current is zero because $`V_{M0}=0`$.

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
| A | $`-4993.723012+j2900.446755`$ | $`0.022393473256-j0.028355992918`$ |
| B | $`4965.448641+j2899.241850`$ | $`-0.021645330351-j0.015401007278`$ |
| C | $`28.274371-j5799.688604`$ | $`-0.000748142905+j0.043757000195`$ |

## 4. Calculate every output field

For each phase, let $`V=x+jy`$, $`I=c+jd`$, and $`S=VI^*=P+jQ`$,
where $`x,y\in\mathbb R`$ are voltage components and $`c,d\in\mathbb R`$ are current
components. The symbol $`c`$ here is a real scalar, not the phase-c label.
Use the following rules with the unrounded phasors in the manual reference ($`\theta_a=0`$).
Section 5 converts the voltage angles with bus 1's phase A as reference, before writing [asym_output.json](asym_output.json).

| Output field | Calculation |
|---|---|
| Node `u`, `u_pu`, `u_angle` | $`\sqrt{x^2+y^2}`$, $`\lvert V\rvert/(U_{\mathrm{rated}}/\sqrt3)`$, $`\mathrm{atan2}(y,x)`$ in radians, followed by the reference conversion in Section 5. |
| Transformer `p_from`, `q_from`, `s_from`, `i_from` | MV $`P=xc+yd`$, $`Q=yc-xd`$, $`\sqrt{P^2+Q^2}`$, and $`\sqrt{c^2+d^2}`$. |
| Transformer `p_to`, `q_to`, `s_to`, `i_to` | Same formulas on LV, equivalently $`P_p`$, $`Q_p`$, $`\sqrt{P_p^2+Q_p^2}`$, and $`\lvert S_p\rvert/U_p`$. |
| Node 1 `p`, `q` and source `p`, `q`, `s`, `i` | Copy the corresponding transformer MV fields. |
| Node 2 `p`, `q` | Copy transformer `p_to`, `q_to`. |
| Load `p`, `q`, `s`, `i` | Negate transformer `p_to`, `q_to` and retain `s_to`, `i_to` magnitudes. |
| Source/load `pf` | Signed active power divided by apparent-power magnitude: `p / s`. |
| Sensor residuals | Measured minus calculated magnitude, active power, or reactive power at the measured terminal. |

The source uses the generator convention and the
load uses the consumer convention.

Representative substitutions:

`u` for MV phase A:

```math
\sqrt{(-4993.723012)^2+(2900.446755)^2}\approx5774.933843\ \mathrm{V}.
```

`u_pu` for MV phase A:

```math
5774.933843378554/(10000/\sqrt3)=1.0002478827080665.
```

Voltage angle for MV phase A:

```math
\arg V_A=\mathrm{atan2}(2900.446755,-4993.723012)
\approx2.61539660449\ \mathrm{rad}.
```

`i_to` for LV phase a:

```math
\sqrt{100^2+10^2}/230=0.43695111396177777\ \mathrm{A}.
```

`i_from` for MV phase A:

```math
\sqrt{0.022393473256^2+(-0.028355992918)^2}
\approx0.036132118383\ \mathrm{A}.
```

Complex power for MV phase A:

```math
\begin{aligned}
S_{MA}&\approx(-4993.723012+j2900.446755)(0.022393473256+j0.028355992918)\\
&\approx-194.071850344-j76.650897532\ \mathrm{VA}.
\end{aligned}
```

`s_from` for MV phase A:

```math
\sqrt{194.071850344^2+76.650897532^2}
\approx208.660593281\ \mathrm{VA}.
```

`source.pf` for phase A:

```math
-194.07185034391784/208.66059328087184=-0.9300838615113276.
```

`load.pf` for phase a:

```math
-100/\sqrt{100^2+10^2}=-0.9950371902099892.
```

The LV voltage base is $`400/\sqrt3=230.94010767585033`$ V. For example,
`node 2.u_pu[a] = 230 / 230.94010767585033 = 0.9959292143521044`.
Convert the LV angles from Section 3 to radians by multiplying by $`\pi/180`$.

The reference residuals are zero because the reconstructed state exactly reproduces the supplied measurements under the stated assumptions.

Transformer loading uses the larger **sum of phase apparent-power magnitudes**:

```math
\texttt{loading}=\frac{\max(\sum_{q\in\{A,B,C\}}|S_{Mq}|,\sum_{p\in\{a,b,c\}}|S_p|)}{S_n}
=\frac{\max(615.224927726205,602.9925372672534)}{100000}
=0.00615224927726205.
```

IDs come from the input. Their values in this case are $`\{1,2,3,4,5,6,7\}`$, with
node IDs $`\{1,2\}`$ identifying MV and LV respectively.

## 5. Convert to PGM's angle reference and store the output

Without voltage-angle measurements, PGM sets phase A of its internal slack bus
to zero. Here that bus is MV node 1. Starting from the manual phasors, define

```math
\theta_{\mathrm{ref}}=\arg V_A
=2.6153966044896544\ \mathrm{rad}
\approx149.8511871901^\circ.
```

Rotate **every voltage and current phasor on both sides** by the same factor:

```math
V^{\mathrm{PGM}}=V e^{-j\theta_{\mathrm{ref}}},\qquad
I^{\mathrm{PGM}}=I e^{-j\theta_{\mathrm{ref}}}.
```

Magnitudes and complex powers are unchanged because
$`V^{\mathrm{PGM}}(I^{\mathrm{PGM}})^*=VI^*`$. The only affected fields stored in this
output are `node.u_angle`. Current outputs contain magnitudes only. Store angles
in radians, wrapped to the principal range:

```math
\texttt{u\_angle}=\arg\!\left(e^{j(\arg V-\theta_{\mathrm{ref}})}\right).
```

