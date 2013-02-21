//
//  AppDelegate.h
//  KARD
//
//  Created by Tyler on 2013-02-19.
//  Copyright (c) 2013 ICAD. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <Wiimote/Wiimote.h>
#import "KPilot.h"

//#import <WiiRemote/WiiRemote.h>
//#import <WiiRemote/WiiRemoteDiscovery.h>

@interface AppDelegate : NSObject <NSApplicationDelegate>

@property (assign) IBOutlet NSWindow *window;
@property (readonly, strong, nonatomic) KPilot * pilot;
//@property (readonly, strong, nonatomic) WiiRemote * wiimote;
//@property (readonly, strong, nonatomic) WiiRemoteDiscovery * wiimoteDiscovery;

@property (readonly, strong, nonatomic) NSPersistentStoreCoordinator *persistentStoreCoordinator;
@property (readonly, strong, nonatomic) NSManagedObjectModel *managedObjectModel;
@property (readonly, strong, nonatomic) NSManagedObjectContext *managedObjectContext;

- (IBAction)saveAction:(id)sender;

@end

Wiimote * wiimote;