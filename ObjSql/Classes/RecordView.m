//
//  CellView.m
//  ObjSql
//
//  Created by John Holdsworth on 13/09/2009.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "RecordView.h"

@implementation RecordView

-(void)drawRect:(CGRect)rect {
	CGFloat colors[3][4];
	colors[0][0] = colors[0][1] = colors[0][2] =
	colors[1][0] = colors[1][1] = colors[1][2] =
	colors[2][0] = colors[2][1] = colors[2][2] = .5+highlighted/2.;
	colors[0][3] = !highlighted ? .08 : .5;
	colors[1][3] = !highlighted ? .20 : .2;
	colors[2][3] = !highlighted ? .30 : .15;
	
	CGContextRef cg = UIGraphicsGetCurrentContext();
	CGColorSpaceRef rgb = CGColorSpaceCreateDeviceRGB();
	CGGradientRef gradient = CGGradientCreateWithColorComponents(rgb, (float *)colors, NULL, sizeof colors/sizeof colors[0]);
	
	CGContextDrawLinearGradient( cg, gradient, CGPointMake( 0, 0 ), CGPointMake( 0, rect.size.height ), kCGGradientDrawsBeforeStartLocation );
	CGGradientRelease(gradient);
	CGColorSpaceRelease(rgb);
}

@end
