To obtain the collected energy (Ecol), mutiply the total number of electrons collected by the Argon w factor and the recombination correction factor (r) as follows:

$$ Ecol = Q \times w \times r $$

where 
- Number of electrons: $Q = \sum \text{hit\_q}$
- Argon w value: $w = 23.6 * 10^{-6}$ [MeV per electrons]
- Recombination factor: $r = 1/0.6$

To obtain the calibrated energy (Ecal), multiply the collected energy by the lifetime correction as follows:

$$ Ecal =  Ecol \times exp\left(\frac{x}{v * t\left(Ecol\right)}\right)$$

where 

- lifetime fit: t(Ecol) = $0.04586717986370039 \times Ecol + 5.278259363488911$
- Drift velocity: $v = 158.2$ [cm/s]
- Drift distance: $x$ [cm]