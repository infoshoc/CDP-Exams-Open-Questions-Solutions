//
//  lock.hpp
//  hw3
//
//  Created by Volodymyr Polosukhin on 12/02/2018.
//  Copyright © 2018 infoshoc. All rights reserved.
//

#ifndef lock_hpp
#define lock_hpp

#define LOCK_MANAGER (0)
#define LOCKS_NUMBER (10)

void dlock(int i);
void dunlock(int i);
void lock_manager();

#endif /* lock_hpp */
