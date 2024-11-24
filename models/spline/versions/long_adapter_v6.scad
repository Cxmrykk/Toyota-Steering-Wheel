/*
    SHAPES
*/
module star(inner_diameter, outer_diameter, steps, height) {
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

module cylinder_extrude($fn, r, h, scale = 1) {
    linear_extrude(height = h, scale = scale)
    polygon(points=[for (i = [0:$fn-1])
        let (angle = i * 360 / $fn)
        [r*cos(angle), r*sin(angle)]
    ]);
}

module circular(diameter, steps, height) {
    cylinder_extrude($fn = steps, r = diameter / 2, h = height);
}

module donut(inner_diameter, outer_diameter, steps, height) {
  difference() {
    circular(outer_diameter, steps, height);
    circular(inner_diameter, steps, height);
  }  
}

/*
    SPLINES
*/

module spline(steps = 20, di = 8.5, do = 6.5, ci = 0.6, co = 0.6, height = 10) {
    union() {
        difference() {
            star(di, do, steps, height);
            rotate(a = 360 / steps / 2) {
                donut(do - co, do + 100, steps, height);
            }
        }
        
        circular(di + ci, steps, height);
    }
}

module big_spline(height) {
    steps = 63;
    
    // between PLA and METAL:
    // 0.3mm outer diameter difference (PLA-, METAL+) 22.6, 22.9
    // 0.2mm inner diameter difference (PLA-, METAL+) 22.4, 22.6
    
    // both lowered 0.3mm (final: 22.8 to 22.5)
    
    // di lowered an additional 0.3mm
    // di = 21.5;
    //di = 20.9;
    di = 21.2;
    
    // do = 23.5;
    //do = 23.2;
    do = 23.5;
    
    // cutoff
    ci = 0.6;
    co = 0.6;
    
    spline(steps, di, do, ci, co, height);
}

module small_spline(height) {
    steps = 33;
    
    // between PLA and METAL:
    // 0.1mm outer difference (PLA-, METAL+)

    
    // with inner 14.7, outer 16.7:
    // 15.2 measured inner diameter
    // 15.9 measured outer diameter
    
    // todo:
    // increase hole by 0.3mm (new measured: 15.5 inner, 16.2 outer)
    // make outer flatter (spline teeth are flat)
    
    // both increased 0.3mm (final: 16.1 to 16.4)
    
    // di lowered an additional 0.3mm (remains the same)
    //di = 14.7;
    di = 14.7;
    
    // do = 16.7;
    do = 17.0;
    
    // cutoff
    ci = 0.6;
    co = 0.6;
    
    spline(steps, di, do, ci, co, height);
}

/*
    ADAPTER

    NOTES TO CONSIDER:

    Spacing
    - The wheel needs spacing (at least 5mm) from the flat plastic (in line with the clockspring)
    - Currently, the spacing is 0mm (scrapes the plastic)
    
    Gradients
*/

module small_shaft() {
    translate([0, 0, 0]) small_spline(12);
}

module big_shaft() {
    translate([0, 0, 2]) big_spline(10);
    rotate(a = 360 / 63 / 2) cylinder_extrude($fn = 63, r = 12, h = 2);
}

// Example usage
rotate([0, 0, 0])
difference() {
    big_shaft();
    small_shaft();
}