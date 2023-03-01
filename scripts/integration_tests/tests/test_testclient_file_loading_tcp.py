#  -------------------------------------------------------------------------
#  Copyright (C) 2014 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

from ramses_test_framework import test_classes
from ramses_test_framework import log
from ramses_test_framework.ramses_test_extensions import with_ramses_process_check


class TestClientFileLoadingTcp(test_classes.OnAllDefaultTargetsTest):
    def __init__(self, methodName='runTest'):
        test_classes.OnAllDefaultTargetsTest.__init__(self, methodName)

    @with_ramses_process_check
    def impl_setUp(self):
        self.ramsesDaemon = self.target.start_daemon()
        self.checkThatApplicationWasStarted(self.ramsesDaemon)
        self.addCleanup(self.target.kill_application, self.ramsesDaemon)
        self.renderer = self.target.start_default_renderer()
        self.checkThatApplicationWasStarted(self.renderer)
        self.addCleanup(self.target.kill_application, self.renderer)
        self.client = self.target.start_client("ramses-test-client", "--test-nr 13 --test-state 0 --cz 5 --folder {}".format(self.target.tmpDir))
        self.checkThatApplicationWasStarted(self.client)
        self.addCleanup(self.target.kill_application, self.client)
        self.percentageOfRGBDifferenceAllowedPerPixel = 0.01

    def impl_tearDown(self):
        self.target.kill_application(self.client)
        self.target.kill_application(self.renderer)
        self.target.kill_application(self.ramsesDaemon)
        log.info("all applications killed")
        self.save_application_output(self.client)
        self.save_application_output(self.renderer)
        self.save_application_output(self.ramsesDaemon)
        log.info("output saved")

    def impl_test(self):
        self.renderer.showScene(37)
        self.validateScreenshot(self.renderer, "testClient_file_loading.png")
