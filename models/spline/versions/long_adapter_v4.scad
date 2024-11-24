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
    
    difference() {
        star(21.5, 23.5, steps); // loses 0.6 on both sides (inner, outer)
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
        star(14.5, 16.5, steps); // loses 0.6 on both sides (inner, outer)
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
    translate([0, 0, 2]) small_spline();
    rotate(a = 360 / 33 / 2) cylinder_extrude($fn = 33, r = 9.25, h = 2);
}

module big_shaft() {
    translate([0, 0, 2]) big_spline();
    rotate(a = 360 / 63 / 2) cylinder_extrude($fn = 63, r = 12, h = 2);
}

rotate([0, 0, 0])
difference() {
    big_shaft();
    small_shaft();
}
