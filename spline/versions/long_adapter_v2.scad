/*
    CONSTANTS
*/
height = 10;

/*
    PRIMITIVE SHAPES
*/
module star(inner_diameter, outer_diameter, steps) {
    r1 = outer_diameter / 2;
    r2 = inner_diameter / 2;
    linear_extrude(height = height) 
        polygon(points=[for (i = [0:2*steps-1]) 
            let (angle = i * 360 / (2*steps)) 
            if (i % 2 == 0) 
                [r1*cos(angle), r1*sin(angle)] 
            else 
                [r2*cos(angle), r2*sin(angle)]
        ]);
}

module circular(diameter, steps) {
    cylinder($fn = steps, r = diameter / 2, h = height);
}

module cylinder_extrude($fn, r, h, scale = 1) {
    linear_extrude(height = h, scale = scale)
    polygon(points=[for (i = [0:$fn-1])
        let (angle = i * 360 / $fn)
        [r*cos(angle), r*sin(angle)]
    ]);
}

module donut(inner_diameter, outer_diameter, steps) {
  difference() {
    circular(outer_diameter, steps);
    circular(inner_diameter, steps);
  }  
}

/*
    SPLINES
*/
module big_spline() {
    steps = 63;
    max_diameter_cutoff = 24;
    
    inner_diameter = 21.5;
    outer_diameter = 23.5;
    cutoff = 0.6;
    
    difference() {
        star(inner_diameter, outer_diameter, steps); // loses 0.6 on both sides (inner, outer)
        rotate(a = 360 / steps / 2) {
            donut(outer_diameter - cutoff, max_diameter_cutoff, steps);
        }
    }
    
    circular(inner_diameter + cutoff, steps);
}

// 15.7 to 15.95
// 0.15 ridged size
// cutoff same (bottom and top)

// distance between wedges:
// 1.5 on the outside
// 1 on the inside

module small_spline() {
    steps = 33;
    max_diameter_cutoff = 20;
    
    // 2.6 = chip_width * 2 + gap
    // ((1.35*33)/(2*pi))*2 = inner + cutoff
    
    // not getting consistant results.
 
    gap = 2.5;
    d1 = 15.45;
    d2 = 15.95;
    flat = (d2 - d1) + 0; // 0.25 +  ??? (changes start from d2 - d1, bottom no change)
    
    cutoff = (2 * (d2 - d1) * flat)/(gap - (3 * flat));
    echo(cutoff)
    echo(d1 - cutoff)
    echo(d2 + cutoff)
    
    difference() {
        star(d1 - cutoff, d2 + cutoff, steps); // loses 0.6 on both sides (inner, outer)
        rotate(a = 360 / steps / 2) {
            donut(d2, max_diameter_cutoff, steps);
        }
    }
    
    cylinder_extrude($fn = steps, r = d1/2, h = height);
}

/*
    ADAPTER
*/

module small_shaft() {
    translate([0, 0, 20]) small_spline();
    translate([0, 0, 14]) rotate(a = 360 / 33 / 2) cylinder_extrude($fn = 33, r = 8 + 0.1, h = 6);
    translate([0, 0, 4])  rotate(a = 360 / 33 / 2) cylinder_extrude($fn = 33, r = 9.25 + 0.1, h = 10, scale = (8 + 0.1)/(9.25 + 0.1));
    rotate(a = 360 / 33 / 2) cylinder_extrude($fn = 33, r = 12 + 0.1, h = 4, scale = (9.25 + 0.1)/(12 + 0.1));
}

module big_shaft() {
    translate([0, 0, 20]) big_spline();
    translate([0, 0, 10]) rotate(a = 360 / 63 / 2) cylinder_extrude($fn = 63, r = 12, h = 10);
    rotate(a = 360 / 63 / 2) cylinder_extrude($fn = 63, r = 15, h = 10, scale = 12/15);
}

difference() {
    big_shaft();
    small_shaft();
}
