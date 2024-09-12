/*
    CONSTANTS
*/

height = 10;

/*
    PRIMATIVE SHAPES
*/

module star(inner_diameter, outer_diameter, steps) {
    r1 = outer_diameter / 2;
    r2 = inner_diameter / 2;
    linear_extrude(height = height) 
        polygon(points=[for (i=[0:2*steps-1]) 
            let(angle = i*360/(2*steps)) 
            if (i%2 == 0) 
                [r1*cos(angle), r1*sin(angle)] 
            else 
                [r2*cos(angle), r2*sin(angle)]
        ]);
}

module circular(diameter, steps) {
    cylinder($fn = steps, r = diameter / 2, h = height);
}

module donut(inner_diameter, outer_diameter, steps) {
  difference() {
    cylinder($fn = steps, r = outer_diameter / 2, h = height);
    cylinder($fn = steps, r = inner_diameter / 2, h = height);
  }  
}

/*
    SPLINES
*/
    
module big_spline() {
    steps = 63;
    max_diameter_cutoff = 24;
    
    difference() {
        star(21.5, 23.5, steps); //loses 0.6 on both sides (inner, outer)
        rotate(a = 360 / steps / 2) {
            donut(22.9, max_diameter_cutoff, steps);
        }
    }
    
    circular(22.1, steps);
}

module small_spline() {
    steps = 33;
    max_diameter_cutoff = 17;
    
    difference() {
        star(14.5, 16.5, steps); //loses 0.6 on both sides (inner, outer)
        rotate(a = 360 / steps / 2) {
            donut(15.9, max_diameter_cutoff, steps);
        }
    }
    
    circular(15.1, steps);
}

/*
    ADAPTER
*/


module small_shaft() {
    translate([0, 0, 20]) small_spline();
    translate([0, 0, 14]) rotate(a = 360 / 33 / 2) cylinder($fn = 33, r = 8, h = 6);
    translate([0, 0, 4])  rotate(a = 360 / 33 / 2) cylinder($fn = 33, r2 = 8, r1 = 9.25, h = 10);
    rotate(a = 360 / 33 / 2) cylinder($fn = 33, r2 = 9.25, r1 = 12, h = 4);
}

module big_shaft() {
    translate([0, 0, 20]) big_spline();
    translate([0, 0, 10]) rotate(a = 360 / 63 / 2) cylinder($fn = 63, r = 12, h = 10);
    rotate(a = 360 / 63 / 2) cylinder($fn = 63, r2 = 12, r1 = 15, h = 10);
}

difference() {
    big_shaft();
    small_shaft();
}


// 63 for large (
// 33 for small

//star(33, 14.5, 16.5, 10);
// 15.5, 15.9
// 22.1, 22.9